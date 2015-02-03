/*
 * Copyright (c) 2013, Altera Corporation.
 * All rights reserved.
 * 
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD 3-Clause license below:
 * 
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 * 
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 * 
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 * 
 *      - Neither Altera nor the names of its contributors may be 
 *         used to endorse or promote products derived from this 
 *         software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */


/* Implementation of all I/O functions except DMA transfers.
 * See aclsoc_dma.c for DMA code.
 */

#include <linux/jiffies.h>
#include <linux/sched.h>
#include "aclsoc.h"


/* ARM does not support readq/writeq but x86-64 does. Use uX as the data type for largest
 * int that can be read/written in one go. readX/writeX are the accessors for this type. */
#ifdef writeq
  #define uX     u64
  #define writeX writeq
  #define readX  readq
#else
  #define uX     u32
  #define writeX writel
  #define readX  readl
#endif

static ssize_t aclsoc_rw_large (void *dev_addr, void __user* use_addr, ssize_t len, char *buffer, int reading);



/* Given (bar_id, device_addr) pair, make sure they're valid and return
 * the resulting address. errno will contain error code, if any. */
void* aclsoc_get_checked_addr (int bar_id, void *device_addr, size_t count,
                               struct aclsoc_dev *aclsoc, ssize_t *errno,
                               int print_error_msg) {

  if (bar_id != 0 && bar_id != 2) {
    ACL_DEBUG (KERN_WARNING "Do not support I/O to BAR %d!", bar_id);
    *errno = -EFAULT;
    return 0;
  }
  /* Make sure the final address is within range */
  if (((unsigned long)device_addr + count) > aclsoc->mapped_region_size[bar_id]) {
    if (print_error_msg) {
      ACL_DEBUG (KERN_WARNING "Requested read/write from BAR #%d from range (%lu, %lu). Length is %zu. BAR length is only %zu!",
                 bar_id, 
                 (unsigned long)device_addr,
                 (unsigned long)device_addr + count, 
                 count,
                 aclsoc->mapped_region_size[bar_id]);
    }
    *errno = -EFAULT;
    return 0;
  }

  *errno = 0;
  return (void*)(aclsoc->mapped_region[bar_id] + (unsigned long)device_addr);  
}


/* Compute address that contains memory window segment control */
static void *get_segment_ctrl_addr (struct aclsoc_dev *aclsoc) {

  void *dev_addr = 0;
  ssize_t errno = 0;
  ssize_t memwindow_cra_offset = (ssize_t)ACL_PCIE_MEMWINDOW_CRA;
  void *ctrl_addr = (void*) (memwindow_cra_offset);
  
  dev_addr = aclsoc_get_checked_addr (2 /*ACL_PCIE_MEMWINDOW_BAR*/, ctrl_addr, sizeof(uX), aclsoc, &errno, 1);
  if (errno != 0) {
    ACL_DEBUG (KERN_DEBUG "ERROR: ctrl_addr %p failed check", ctrl_addr);
    return NULL;
  } else {
    return dev_addr;
  }
}


static void aclsoc_set_segment_by_val (struct aclsoc_dev *aclsoc, uX new_val) {

  void *ctrl_addr =  aclsoc->global_mem_segment_addr;
  if (ctrl_addr == NULL) {
    return;
  }
  return;
  
  if (new_val != aclsoc->global_mem_segment) {
    writeX (new_val, ctrl_addr);
    aclsoc->global_mem_segment = new_val;
  }
  ACL_VERBOSE_DEBUG (KERN_DEBUG " Changed global memory segment to %llu.", new_val);
}


/* Response to user's open() call */
int aclsoc_open(struct inode *inode, struct file *file) {

  struct aclsoc_dev *aclsoc = 0;
  int result = 0;

  /* pointer to containing data structure of the character device inode */
  aclsoc = container_of(inode->i_cdev, struct aclsoc_dev, cdev);
  
  if (down_interruptible(&aclsoc->sem)) {
    return -ERESTARTSYS;
  }
  
  /* create a reference to our device state in the opened file */
  file->private_data = aclsoc;
  ACL_DEBUG (KERN_DEBUG "aclsoc = %p, pid = %d (%s)", 
             aclsoc, current->pid, current->comm); 

  aclsoc->user_pid = current->pid;
  aclsoc->user_task = current;
  
  aclsoc->global_mem_segment = 0;
  aclsoc->saved_kernel_irq_mask = 0;
  aclsoc->global_mem_segment_addr = get_segment_ctrl_addr(aclsoc);
  
#if 0
  if (aclsoc->user_pid == -1) {
    aclsoc->user_pid = current->pid;
  } else {
    ACL_DEBUG (KERN_WARNING "Tried open() by pid %d. Already opened by %d", current->pid, aclsoc->user_pid);
    result = -EFAULT;
    goto done;
  }
#endif

  #if !POLLING
  load_signal_info (aclsoc);
  if (aclsoc->user_task == NULL) {
    ACL_DEBUG (KERN_WARNING "Tried open() by pid %d but couldn't find associated task_info", current->pid);
    result = -EFAULT;
    goto done;
  }
  #endif
  
  result = 0;

#if !POLLING
done:
#endif

  up (&aclsoc->sem);
  return result;
}


/* Response to user's close() call. Will also be called by the kernel
 * if the user process dies for any reason. */
int aclsoc_close(struct inode *inode, struct file *file) {

  ssize_t result = 0;
  struct aclsoc_dev *aclsoc = (struct aclsoc_dev *)file->private_data;
  ACL_DEBUG (KERN_DEBUG "aclsoc = %p, pid = %d, dma_idle = %d",
             aclsoc, current->pid, 1 /*, aclsoc_dma_get_idle_status(aclsoc)*/ ); 
  
  if (down_interruptible(&aclsoc->sem)) {
    return -ERESTARTSYS;
  }
  
#if 0  
  if (aclsoc->user_pid == current->pid) {
    aclsoc->user_pid = -1;
  } else {
    ACL_DEBUG (KERN_WARNING "Tried close() by pid %d. Opened by %d", current->pid, aclsoc->user_pid);
    result = -EFAULT;
    goto done;
  }
#endif

  /* All mmap'ed memory will be automatically released when
   * the user process terminates. */

  up (&aclsoc->sem);
  return result;
}


/* Read a small number of bytes and put them into user space */
ssize_t aclsoc_read_small (void *read_addr, void __user* dest_addr, ssize_t len) {

  ssize_t copy_res = 0;
  switch (len) {
  case 1: {
    u8 d = readb ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  case 2: {
    u16 d = readw ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  case 4: {
    u32 d = readl ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }

#ifdef writeq
  case 8: {
    u64 d = readq ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
#endif

  default:
    break;
  }

  if (copy_res) {
    return -EFAULT;
  } else {
    return 0;
  }
}


/* Write a small number of bytes taken from user space */
ssize_t aclsoc_write_small (void *write_addr, void __user* src_addr, ssize_t len) {

  ssize_t copy_res = 0;
  switch (len) {
  case 1: {
    u8 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writeb ( d, write_addr );
    break;
  }
  case 2: {
    u16 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writew ( d, write_addr );
  }
  case 4: {
    u32 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writel ( d, write_addr );
    break;
  }
  
#ifdef writeq
  case 8: {
    u64 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writeq ( d, write_addr );
    break;
  }
#endif

  default:
    break;
  }

  if (copy_res) {
    return -EFAULT;
  } else {
    return 0;
  }
}



/* Read or Write arbitrary length sequency starting at read_addr and put it into
 * user space at dest_addr. if 'reading' is set to 1, doing the read. If 0, doing
 * the write. */
static ssize_t aclsoc_rw_large (void *dev_addr, void __user* user_addr,
                                  ssize_t len, char *buffer, int reading) {
  size_t bytes_left = len;
  size_t i, num_missed;
  uX *ibuffer = (uX*)buffer;
  char *cbuffer;
  size_t offset, num_to_read;
  size_t chunk = BUF_SIZE;
  
  u64 startj, ej;
  u64 sj = 0, acc_readj = 0, acc_transfj = 0;
  
  startj = get_jiffies_64();
  
  /* Reading upto BUF_SIZE values, one int at a time, and then transfer
   * the buffer at once to user space. Repeat as necessary. */
  while (bytes_left > 0) {
    if (bytes_left < BUF_SIZE) {
      chunk = bytes_left;
    } else {
      chunk = BUF_SIZE;
    }
    
    if (!reading) {
      sj = get_jiffies_64();
      if (copy_from_user (ibuffer, user_addr, chunk)) {
        return -EFAULT;
      }
      acc_transfj += get_jiffies_64() - sj;
    }
    
    /* Read one uX at a time until fill the buffer. Then copy the whole
     * buffer at once to user space. */
    sj = get_jiffies_64();
    num_to_read = chunk / sizeof(uX);
    for (i = 0; i < num_to_read; i++) {
      if (reading) {
        ibuffer[i] = readX ( ((uX*)dev_addr) + i);
      } else {
        writeX ( ibuffer[i], ((uX*)dev_addr) + i );
      }
    }
    
    /* If length is not a multiple of sizeof(uX), will miss last few bytes.
     * In that case, read it one byte at a time. This can only happen on 
     * last iteration of the while() loop. */
    offset = num_to_read * sizeof(uX);
    num_missed = chunk - offset;
    cbuffer = (char*)(ibuffer + num_to_read);
    
    for (i = 0; i < num_missed; i++) {
      if (reading) {
        cbuffer[i] = readb ( (u8*)(dev_addr) + offset + i );
      } else {
        writeb ( cbuffer[i], (u8*)(dev_addr) + offset + i );
      }
    }
    acc_readj += get_jiffies_64() - sj;
    
    if (reading) {
      sj = get_jiffies_64();
      if (copy_to_user (user_addr, ibuffer, chunk)) {
        return -EFAULT;
      }
      acc_transfj += get_jiffies_64() - sj;
    }
    
    dev_addr += chunk;
    user_addr += chunk;
    bytes_left -= chunk;
  }
  
  ej = get_jiffies_64();
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Spent %u msec %sing %lu bytes", jiffies_to_msecs(ej - startj), 
                          reading ? "read" : "writ", len);
  ACL_VERBOSE_DEBUG (KERN_DEBUG "  Dev access %u msec. User space transfer %u msec",
                        jiffies_to_msecs(acc_readj),
                        jiffies_to_msecs(acc_transfj));
  return 0;
}

/* Set CRA window so raw_user_ptr is "visible" to the BAR.
 * Return pointer to use to access the user memory */
void* aclsoc_set_segment (struct aclsoc_dev *aclsoc, void * raw_user_ptr) {

  //ssize_t cur_segment = ((ssize_t)raw_user_ptr) / ACL_PCIE_MEMWINDOW_SIZE;  
  ssize_t cur_segment = ((ssize_t)raw_user_ptr) & ((size_t)1 - (ACL_PCIE_MEMWINDOW_SIZE-1));
  aclsoc_set_segment_by_val (aclsoc, cur_segment);  

  /* Can use the return value in all read/write functions in this file now */
  return (void*)((ssize_t)ACL_PCIE_MEMWINDOW_BASE + ((ssize_t)raw_user_ptr % ACL_PCIE_MEMWINDOW_SIZE));
}


/* Both start and end, user and device addresses must be 
 * 64-byte aligned to use DMA */
int aligned_request (struct acl_cmd *cmd, size_t count) {
  
  return (( (unsigned long)cmd->user_addr   & DMA_ALIGNMENT_BYTE_MASK) | 
          ( (unsigned long)cmd->device_addr & DMA_ALIGNMENT_BYTE_MASK) |
          ( count                           & DMA_ALIGNMENT_BYTE_MASK)
         ) == 0;
}                           


/* High-level read/write dispatcher. */
ssize_t aclsoc_rw(struct file *file, char __user *buf, 
                  size_t count, loff_t *pos,
                  int reading) {
  
  struct aclsoc_dev *aclsoc = (struct aclsoc_dev *)file->private_data;
  struct acl_cmd __user *ucmd;
  struct acl_cmd kcmd;
  u64 old_segment = 0;
  int restore_segment = 0;
  void *addr = 0;
  int aligned = 0;
  int use_dma = 0;
  size_t size = 0;
  ssize_t result = 0;
  ssize_t errno = 0;
  
  if (down_interruptible(&aclsoc->sem)) {
    return -ERESTARTSYS;
  }
  
  ucmd = (struct acl_cmd __user *) buf;
  if (copy_from_user (&kcmd, ucmd, sizeof(*ucmd))) {
		result = -EFAULT;
    goto done;
	}

  size = kcmd.size;
 
  if (kcmd.bar_id == ACLPCI_CMD_BAR) {
    /* This is not a read but a special command. */
    result = aclsoc_exec_cmd (aclsoc, kcmd, size);
    goto done;
  }

  if (kcmd.bar_id == 0 && 
       ((ssize_t)kcmd.device_addr >= ACL_PCIE_MEMWINDOW_BASE && 
        (ssize_t)kcmd.device_addr + size <= ACL_PCIE_MEMWINDOW_BASE + ACL_PCIE_MEMWINDOW_SIZE)) {
    /* Writes to global memory go to [0x10000, 0x20000) unchanged. */
  } else {
    /* All other memory accesses are controls, which go via the 
     * light-weight HPS2FPGA bridge (bar 2). */
    kcmd.bar_id = 2;
  }
  
  /* Only using DMA for large aligned reads/writes on global memory
   * (due to some assumptions inside the DMA hardware). */
  aligned = aligned_request (&kcmd, size);
  use_dma = USE_DMA && (size >= 1024) && 
            aligned && kcmd.bar_id == ACLPCI_DMA_BAR;
  
  if (!use_dma) {
    /* Do bounds checking on addresses, for DMA we don't know memory size */
    if (kcmd.bar_id != ACLPCI_DMA_BAR) {
      addr = aclsoc_get_checked_addr (kcmd.bar_id, kcmd.device_addr, size, aclsoc, &errno, 1);
    }
    else {
      /* If not using DMA, but command specifies addresses in DMA's address
       * space, we need to translate these to accesses to the memwindow.  The
       * user-space written HAL currently also does this so we need to restore
       * the current segment in hardware. */

      ACL_DEBUG (KERN_DEBUG "For global memory accesses, trying to change segment so the address is mapped into PCIe BAR");
      old_segment = aclsoc->global_mem_segment;
      restore_segment = 1;
      kcmd.bar_id = ACL_PCIE_MEMWINDOW_BAR;
      kcmd.device_addr = aclsoc_set_segment (aclsoc, kcmd.device_addr);
      addr = aclsoc_get_checked_addr (kcmd.bar_id, kcmd.device_addr, size, aclsoc, &errno, 1);
    }

    if (errno != 0) {
      result = -EFAULT;
      goto done;
    }
  }
  
  /* Offset value is always an address offset, not element offset. */
  switch (size) {
  case 1:
  case 2:
  case 4:
#ifdef writeq
  case 8:
#endif
  {
    if (reading) {
      result = aclsoc_read_small (addr, (void __user*) kcmd.user_addr, size);
    } else {
      result = aclsoc_write_small (addr, (void __user*) kcmd.user_addr, size);
    }
    break;
  }
    
  default:
    if (use_dma) {
      // result = aclsoc_dma_rw (aclsoc, kcmd.device_addr, (void __user*) kcmd.user_addr, size, reading);
    } else {
      result = aclsoc_rw_large (addr, (void __user*) kcmd.user_addr, size, aclsoc->buffer, reading);
    }
    break;
  }
  
  /* If had to change the segment to get this read through, restore the value */
  if (restore_segment) {
    ACL_DEBUG (KERN_DEBUG "Restoring mem segment to %llu", old_segment);
    aclsoc_set_segment_by_val (aclsoc, old_segment);
  }
  
done:
  up (&aclsoc->sem);
  return result;
}


/* Response to user's read() call */
ssize_t aclsoc_read(struct file *file, char __user *buf, 
                    size_t count, loff_t *pos) {

  return aclsoc_rw (file, buf, count, pos, 1 /* reading */);
}


/* Response to user's write() call */
ssize_t aclsoc_write(struct file *file, const char __user *buf, 
                     size_t count, loff_t *pos) {

  return aclsoc_rw (file, (char __user *)buf, count, pos, 0 /* writing */);
}

