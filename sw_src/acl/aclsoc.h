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

/* Global declarations shared by all files of this driver. */

#ifndef ACLPCI_H
#define ACLPCI_H


#include <linux/kobject.h>
#include <linux/kdev_t.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kref.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>


/* includes from opencl/include/pcie */
#include "hw_pcie_constants.h"
#include "pcie_linux_driver_exports.h"

/* Local includes */
#include "version.h"


/* For now, disable DMA on big-endian system. */
#ifdef ACLPCI_BIGENDIAN
#define USE_DMA           0
#else
#define USE_DMA           0
#endif

//#include "aclsoc_dma.h"


#define DRIVER_NAME       "aclsoc"
#define BOARD_NAME        "acl"


/* Set to 1 to use Polling (instead of interrupts) to communicate
 * with hal. HAL must be compiled in the same mode */
#define POLLING           0


/* Debugging defines */
#define VERBOSE_DEBUG     0
#define ACL_DEBUG(...)					\
  do {						\
    printk(KERN_DEBUG "%s (%d): ", __func__, __LINE__);	\
    printk(__VA_ARGS__);			\
    printk(KERN_DEBUG "\n");				\
  } while (0)
  
#if VERBOSE_DEBUG
# define ACL_VERBOSE_DEBUG(...) ACL_DEBUG(__VA_ARGS__)
#else 
# define ACL_VERBOSE_DEBUG(...)
#endif

/* Don't actually bring down the kernel on an error condition */
#define assert(expr) \
do { \
   if (!(expr)) { \
      printk(KERN_ERR "Assertion failed! %s, %s, %s, line %d\n", \
            #expr, __FILE__, __func__, __LINE__); \
   } \
} while (0)



/* Maximum size of driver buffer (allocated with kalloc()).
 * Needed to copy data from user to kernel space, among other
 * things. */
static const size_t BUF_SIZE = PAGE_SIZE;

struct addr_map_elem {
  unsigned long vm_start;
  size_t size;
  void *cpu_addr;
  dma_addr_t dma_handle;
  struct kref ref;
};

#define MAX_ADDR_MAP_ENTRIES (PAGE_SIZE/sizeof(struct addr_map_elem))


/* Device data used by this driver. */
struct aclsoc_dev {
  
  char *mapped_region[3];
  size_t mapped_region_size[3];
  
  /* Used for doing large physically-contiguous memory allocations
   * requested by user via mmap */
  struct addr_map_elem *addr_map;
    
  /* Controls which section of board's DDR maps to BAR */
  u64 global_mem_segment;
  
  /* Kernel irq - mustn't assume it's safe to enable kernel irq */
  char saved_kernel_irq_mask;
  
  /* Location of global_mem_segment value on the board. */
  void *global_mem_segment_addr;
  
  /* temporary buffer. If allocated, will be BUF_SIZE. */
  char *buffer;
  
  /* Mutex for this device. */
  struct semaphore sem;
  
  /* PID of process that called open() */
  int user_pid;
  
  /* character device */
  dev_t cdev_num;
  struct cdev cdev;
  struct class *my_class;
  struct device *device;


  /* signal sending structs */
  struct siginfo signal_info;
  struct task_struct *user_task;
  int user_filehandle;
 
  /* Debug data */  
  /* number of hw interrupts handled. */
  size_t num_handled_interrupts;
  size_t num_undelivered_signals;
};


/* aclsoc_fileio.c funciton */
int aclsoc_open(struct inode *inode, struct file *file);
int aclsoc_close(struct inode *inode, struct file *file);
ssize_t aclsoc_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
ssize_t aclsoc_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);
void* aclsoc_get_checked_addr (int bar_id, void *device_addr, size_t count,
                               struct aclsoc_dev *aclsoc, ssize_t *errno, int print_error_msg);

/* aclsoc.c functions */
void load_signal_info (struct aclsoc_dev *aclsoc);
int init_irq (struct aclsoc_dev *dev_id);
void release_irq (struct aclsoc_dev *aclsoc);
void unmask_kernel_irq(struct aclsoc_dev *aclsoc);


/* acl_cmd.c functions */
ssize_t aclsoc_exec_cmd (struct aclsoc_dev *aclsoc, struct acl_cmd kcmd, size_t count);

#endif /* ACLPCI_H */

