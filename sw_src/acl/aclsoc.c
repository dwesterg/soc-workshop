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

/* Top-level file for the driver.
 * Deal with device init and shutdown, BAR mapping, and interrupts. */

#include "aclsoc.h"
#include <asm/siginfo.h>    //siginfo
#include <linux/rcupdate.h> //rcu_read_lock
#include <linux/version.h>  //kernel_version

MODULE_AUTHOR  ("Dmitry Denisenko");
MODULE_LICENSE ("Dual BSD/GPL");
MODULE_DESCRIPTION ("Driver for Altera OpenCL SoC Acceleration Devices");
MODULE_SUPPORTED_DEVICE ("Altera OpenCL SoC Devices");


// all these defines should be absorbed by acl_init

// Offsets taken from CV Device handbook, Chapter 3, page 1-16
// Sizes decided by the QSYS system.
#define lwHPS2FPGA_OFFSET   0xFF200000
#define lwHPS2FPGA_SIZE     0x40000   // 2 MB

#define   HPS2FPGA_OFFSET   0xC0000000 
#define   HPS2FPGA_SIZE     0x10000   // Global memory window size

// FPGA_IRQ0 interrupt is at 72.
#define PIO_IRQ (72)


/* Static function declarations */
static int __init aclsoc_probe(void);
static int __init init_chrdev (struct aclsoc_dev *aclsoc);
static void __exit aclsoc_remove(void);
static int aclsoc_mmap (struct file *filp, struct vm_area_struct *vma);

static struct aclsoc_dev *aclsoc = 0;
static struct class *aclsoc_class = NULL;
char* irq_virtual_deassert_remapped_base;


/*
static struct platform_driver aclsoc_driver = {
  .driver = {
    .name = DRIVER_NAME,
    .owner = THIS_MODULE,
  },
  .probe = NULL,
  .remove = NULL, // FPGA is always there. So no probe/remove
};
*/

struct file_operations aclsoc_fileops = {
  .owner =    THIS_MODULE,
  .read =     aclsoc_read,
  .write =    aclsoc_write,
/*  .ioctl =    aclsoc_ioctl, */
  .open =     aclsoc_open,
  .release =  aclsoc_close,
  .mmap    = aclsoc_mmap,
};




/* Allocate /dev/BOARD_NAME device */
static int __init init_chrdev (struct aclsoc_dev *aclsoc) {

  int dev_minor =   0;
  int dev_major =   0; 
  int devno = -1;

  /* request major number for device */
  int result = alloc_chrdev_region(&aclsoc->cdev_num, dev_minor, 1 /* one device*/, BOARD_NAME);
  dev_major = MAJOR(aclsoc->cdev_num);
  if (result < 0) {
    ACL_DEBUG (KERN_WARNING "can't get major ID %d", dev_major);
    goto fail_alloc;
  }
  
  aclsoc_class = class_create(THIS_MODULE, DRIVER_NAME);
  if (IS_ERR(aclsoc_class)) {
    printk(KERN_ERR "aclsoc: can't create class\n");
    goto fail_class;
  }
  
  devno = MKDEV(dev_major, dev_minor);
    
  cdev_init (&aclsoc->cdev, &aclsoc_fileops);
  aclsoc->cdev.owner = THIS_MODULE;
  aclsoc->cdev.ops = &aclsoc_fileops;
  result = cdev_add (&aclsoc->cdev, devno, 1);
  /* Fail gracefully if need be */
  if (result) {
    printk(KERN_NOTICE "Error %d adding aclsoc (%d, %d)", result, dev_major, dev_minor);
    goto fail_add;
  }
  ACL_DEBUG (KERN_DEBUG "aclsoc = %d:%d", MAJOR(devno), MINOR(devno));
  
  /* create device nodes under /dev/ using udev */
  aclsoc->device = device_create(aclsoc_class, NULL, devno, NULL, BOARD_NAME "%d", dev_minor);
  if (IS_ERR(aclsoc->device)) {
    printk(KERN_NOTICE "Can't create device\n");
    goto fail_dev_create;
  }
  return 0;
  
/* ERROR HANDLING */
fail_dev_create:
  cdev_del(&aclsoc->cdev);
  
fail_add:
  class_destroy(aclsoc_class);

fail_class:
  /* free the dynamically allocated character device node */
  unregister_chrdev_region(devno, 1/*count*/);
  
fail_alloc:
  return -1;
}


/* Returns virtual mem address corresponding to location of IRQ control
 * register of the board */
static void* get_interrupt_enable_addr(struct aclsoc_dev *aclsoc) {

  /* Bar 2, register PCIE_CRA_IRQ_ENABLE is the IRQ enable register
   * (among other things). */
  return (void*)(aclsoc->mapped_region[2 /*ACL_PCI_CRA_BAR*/] + 
                 (unsigned long)(ACL_KERNEL_CSR_OFFSET + PCIE_CRA_IRQ_ENABLE));
}


/* Disable interrupt generation on the device. */
static void mask_irq(struct aclsoc_dev *aclsoc) {

  writel (0x0, get_interrupt_enable_addr(aclsoc));
}


/* Enable interrupt generation on the device. */
static void unmask_irq(struct aclsoc_dev *aclsoc) {

  writel (0x1, get_interrupt_enable_addr(aclsoc));
}

/* Enable interrupt generation on the device. */
void unmask_kernel_irq(struct aclsoc_dev *aclsoc) {

  writel (0x1, get_interrupt_enable_addr(aclsoc));
}


irqreturn_t aclsoc_irq (int irq, void *dev_id) {

  struct aclsoc_dev *aclsoc = (struct aclsoc_dev *)dev_id;
  u32 irq_status;
  irqreturn_t res;
  unsigned int kernel_update = 0, dma_update = 0;
  
   
  if (aclsoc == NULL) {
    return IRQ_NONE;
  }
  
  /* From this point on, this is our interrupt. So return IRQ_HANDLED
   * no matter what (since nobody else in the system will handle this
   * interrupt for us). */
  aclsoc->num_handled_interrupts++;
  
  /* If using old-style interrupts (dedicated wire), bring it down ASAP.
   * Otherwise, will get a flood of interrupts */
  mask_irq(aclsoc);

  
  /* Kernel and DMA interrupts are on separate lines. Since DMA is not working,
   * can only get a kernel-done interrupt. */
  irq_status = 0x1;
  kernel_update = 1;
  dma_update = 0;

  if (kernel_update) {
    #if !POLLING
      /* Send SIGNAL to user program to notify about the kernel update interrupt. */
      if (aclsoc->user_task != NULL) {
        int ret = send_sig_info(SIG_INT_NOTIFY, &aclsoc->signal_info, aclsoc->user_task);      
        if (ret < 0) {
          /* Can get to this state if the host is suspended for whatever reason.
           * Just print a warning message the first few times. The FPGA will keep
           * the interrupt level high until the kernel done bit is cleared (by the host).
           * See Case:84460. */
          aclsoc->num_undelivered_signals++;
          if (aclsoc->num_undelivered_signals < 5) {
            ACL_DEBUG (KERN_DEBUG "Error sending signal to host! irq_status is 0x%x\n", irq_status);
          }
        }
      }
    #else
       ACL_VERBOSE_DEBUG (KERN_WARNING "Kernel update interrupt. Letting host POLL for it.");
    #endif
    res = IRQ_HANDLED;
     
  }
  if (dma_update) {
    /* A DMA-status interrupt - let the DMA object handle this without going to
      * user space */
    ACL_DEBUG (KERN_WARNING "DMA interrupt!?\n");
    // res = aclsoc_dma_service_interrupt(aclsoc);
  
  }
  if (!kernel_update && !dma_update) {
    ACL_VERBOSE_DEBUG (KERN_WARNING "Our interrupt is neither for DMA nor for Kernel! irq_status is 0x%x\n", irq_status);
    res = IRQ_HANDLED;
  }
  
  /* Unmasking interrupts will be done by the host when it deals with the current one. */
    
  return res;
}


void load_signal_info (struct aclsoc_dev *aclsoc) {

  /* Setup siginfo struct to send signal to user process. Doing it once here
   * so don't waste time inside the interrupt handler. */
  struct siginfo *info = &aclsoc->signal_info;
  memset(info, 0, sizeof(struct siginfo));
  info->si_signo = SIG_INT_NOTIFY;
  /* this is bit of a trickery: SI_QUEUE is normally used by sigqueue from user
   * space,  and kernel space should use SI_KERNEL. But if SI_KERNEL is used the
   * real_time data is not delivered to the user space signal handler function. */
  info->si_code = SI_QUEUE;
  info->si_int = 0;  /* Signal payload. Will be filled later with 
                        ACLPCI_CMD_SET_SIGNAL_PAYLOAD cmd from user. */
}


int init_irq (struct aclsoc_dev *aclsoc) {

  u32 irq_type;
  int rc;
  
#if POLLING
  return 0;
#endif

  if (aclsoc == NULL) {
    ACL_DEBUG (KERN_DEBUG "Invalid inputs to init_irq (%p)", aclsoc);
    return -1;
  }

  /* Using non-shared MSI interrupts.*/
  irq_type = 0;
  
  rc = request_irq (PIO_IRQ, aclsoc_irq, irq_type, DRIVER_NAME, (void*)aclsoc);
  if (rc) {
    ACL_DEBUG (KERN_DEBUG "Could not request IRQ #%d, error %d", PIO_IRQ, rc);
    return -1;
  }
  ACL_DEBUG (KERN_DEBUG "Succesfully requested IRQ #%d", PIO_IRQ);
  
  aclsoc->num_handled_interrupts = 0;
  aclsoc->num_undelivered_signals = 0;
  
  // aclsoc_dma_init(aclsoc);
  
  /* Enable interrupts */
  unmask_irq(aclsoc);
  
  return 0;
}


void release_irq (struct aclsoc_dev *aclsoc) {

  int num_usignals;

#if POLLING
  return;
#endif
  
  // aclsoc_dma_finish(aclsoc);
  
  /* Disable interrupts before going away. If something bad happened in
   * user space and the user program crashes, the interrupt assigned to the device
   * will be freed (on automatic close()) call but the device will continue 
   * generating interrupts. Soon the kernel will notice, complain, and bring down
   * the whole system. */
  mask_irq(aclsoc);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Freeing IRQ %d", PIO_IRQ);
  free_irq (PIO_IRQ, (void*)aclsoc);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Handled %d interrupts", 
        aclsoc->num_handled_interrupts);
        
  num_usignals = aclsoc->num_undelivered_signals;
  if (num_usignals > 0) {
    ACL_DEBUG (KERN_DEBUG "Number undelivered signals is %d", num_usignals);
  }
    
#if 0
  /* Perform software reset on the FPGA.
   * If the host is killed after launching a kernel but before the kernel
   * finishes, the FPGA will keep sending "kernel done" interrupt. That might
   * kill a *new* host before it can do anything. 
   *
   * WARNING: THIS RESET LOGIC IS ALSO IN THE HAL/PCIE.
   *          IF YOU CHANGE IT, UPDATE THE HAL AS WELL!!! */
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Reseting kernel on FPGA");
  //pio_out_addr_base = ((struct aclsoc_dev*)aclsoc)->mapped_region[ACL_PCIE_PIO_OUT_BAR] + ACL_PCIE_PIO_OUT_OFFSET - ACL_PCIE_MEMWINDOW_SIZE;
  /* Do the reset */
  //writel (ACL_PCIE_GET_BIT(PIO_OUT_SWRESET), pio_out_addr_base + PIO_SET);
  /* De-assert the reset */
  //for (i = 0; i < 10; i++) {
  //  writel (ACL_PCIE_GET_BIT(PIO_OUT_SWRESET), pio_out_addr_base + PIO_CLR);
  //}
#endif

  mask_irq(aclsoc);
}


static void free_map_info (struct addr_map_elem *info) 
{
  if (info->vm_start != 0) {
    ACL_DEBUG (KERN_DEBUG "free_map_info on vaddr %p, dma 0x%x\n", info->cpu_addr, info->dma_handle);
    dma_free_coherent(NULL, 
            info->size, 
            info->cpu_addr,
            info->dma_handle);
    memset (info, 0, sizeof(*info) * 1);
  }
}

static void aclsoc_release_mmap_memory (struct kref *ref)
{
  struct addr_map_elem *info = container_of(ref, struct addr_map_elem, ref);
  free_map_info (info);
}

/* aclsoc_vma_open and _close will are called during mmap/munmp
 * operation. There are no direct call to these functions in this
 * driver. */
static void aclsoc_vma_open (struct vm_area_struct *vma)
{
  struct addr_map_elem *info = vma->vm_private_data;
  kref_get(&info->ref);
}

static void aclsoc_vma_close (struct vm_area_struct *vma)
{
  struct addr_map_elem *info = vma->vm_private_data;
  kref_put(&info->ref, aclsoc_release_mmap_memory);
}

static struct vm_operations_struct aclsoc_vm_ops = {
  .open =  aclsoc_vma_open,
  .close = aclsoc_vma_close,
};


static int aclsoc_mmap (struct file *filp, struct vm_area_struct *vma) {
  
  int iMapInfo;
  size_t size = vma->vm_end - vma->vm_start;
  size_t allocated_size = size;
  void *kalloc_memory = NULL;
  dma_addr_t dma_handle;
  struct addr_map_elem *info = NULL;
  
  /* Make sure we have space to store this allocation */
  for (iMapInfo = 0; iMapInfo < MAX_ADDR_MAP_ENTRIES; iMapInfo++) {
    if (aclsoc->addr_map[iMapInfo].vm_start == 0) break;
  }
  if (iMapInfo == MAX_ADDR_MAP_ENTRIES) {
    printk (KERN_DEBUG "Out of addr_map buffers!\n");
    return -ENOMEM;
  }
  info = &(aclsoc->addr_map[iMapInfo]);
  
  kalloc_memory = dma_alloc_coherent(NULL, allocated_size, &dma_handle, GFP_KERNEL);
  if (kalloc_memory == NULL) {
    return -ENOMEM;
  }
  
  // kmalloc returns "kernel logical addresses".
  // __pa()          maps "kernel logical addresses" to "physical addresses".
  // remap_pfn_range maps "physical addresses" to "user virtual addresses".
  // kernel logical addresses are usually just physical addresses with an offset.
  // Make the pages uncache-able. Otherwise, will run into consistency issues.
  vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
  if (remap_pfn_range(vma, vma->vm_start,
                      dma_handle >> PAGE_SHIFT,
                      size,
                      vma->vm_page_prot) < 0) {
    return -EAGAIN;
  }
  
  info->vm_start = vma->vm_start;
  info->size = allocated_size;
  info->cpu_addr = kalloc_memory;
  info->dma_handle = dma_handle;
  kref_init(&info->ref);

  vma->vm_ops = &aclsoc_vm_ops;
  vma->vm_private_data = info;
  
  return 0;
}

/* Free all DMA buffers here, just in case the user forgot some */
void free_contiguous_memory(struct aclsoc_dev *aclsoc) {

  int iMapInfo;
  for (iMapInfo = 0; iMapInfo < MAX_ADDR_MAP_ENTRIES; iMapInfo++) {
    free_map_info(&(aclsoc->addr_map[iMapInfo]));
  }
}


static int __init aclsoc_probe(void) {

  int res, i;
  ACL_VERBOSE_DEBUG (KERN_DEBUG " probe");

  // That's static aclsoc -- this driver is only for one instance of the device!  
  aclsoc = kzalloc(sizeof(struct aclsoc_dev), GFP_KERNEL);
  if (!aclsoc) {
    ACL_DEBUG(KERN_WARNING "Couldn't allocate memory!\n");
    goto fail_kzalloc;
  }
  
  sema_init (&aclsoc->sem, 1);
  //aclsoc->pci_dev = dev;
  //platform_set_drvdata(dev, (void*)aclsoc);
  aclsoc->user_pid = -1;
  
  aclsoc->addr_map = kzalloc(sizeof(struct addr_map_elem) * MAX_ADDR_MAP_ENTRIES, GFP_KERNEL);
  aclsoc->buffer = kmalloc (BUF_SIZE * sizeof(char), GFP_KERNEL);
  if (!aclsoc->buffer) {
    ACL_DEBUG(KERN_WARNING "Couldn't allocate memory for buffer!\n");
    goto fail_kmalloc;
  }
  
  res = init_chrdev (aclsoc);
  if (res) {
    goto fail_chrdev_init;
  }

  // region 0 for global memory, region 2 for control, just like BARs for 
  // PCIe devices. Makes acl_init.c simpler.
  
  // all control traffic uses slow lwHPS2FGPA bridge
  aclsoc->mapped_region[2] = ioremap (lwHPS2FPGA_OFFSET, lwHPS2FPGA_SIZE);
  aclsoc->mapped_region_size[2] = lwHPS2FPGA_SIZE;

  aclsoc->mapped_region[1] = (void*)0;
  aclsoc->mapped_region_size[1] = 0;
  
  // all data traffic uses wide HPS2FPGA bridge
  aclsoc->mapped_region[0] = ioremap (HPS2FPGA_OFFSET, 2 * HPS2FPGA_SIZE);
  aclsoc->mapped_region_size[0] = 2 * HPS2FPGA_SIZE;
  
  for (i = 0; i < 3; i+=2) {
    printk(KERN_DEBUG "mapped region %d (lw) to [%p, %p). Size = %zu\n", 
          i,
          aclsoc->mapped_region[i],
          aclsoc->mapped_region[i] + aclsoc->mapped_region_size[i],
          aclsoc->mapped_region_size[i]);
  }
  
#if !POLLING
  return init_irq (aclsoc);
#else
  return 0;
#endif

/* ERROR HANDLING */
fail_chrdev_init:
  kfree (aclsoc->buffer);
  
fail_kmalloc:
  kfree (aclsoc);
  
fail_kzalloc:
  return -1;
}


static void __exit aclsoc_remove(void) {

  ACL_DEBUG (KERN_DEBUG ": aclsoc is %p", aclsoc);
  if (aclsoc == NULL) {
    return;
  }
  
  #if !POLLING
  release_irq (aclsoc);
  #endif

  device_destroy(aclsoc_class, aclsoc->cdev_num);
  class_destroy(aclsoc_class);
  cdev_del (&aclsoc->cdev);
  unregister_chrdev_region (aclsoc->cdev_num, 1);  
  
  iounmap (aclsoc->mapped_region[0]);
  iounmap (aclsoc->mapped_region[2]);
  
  kfree (aclsoc->buffer);
  
  free_contiguous_memory(aclsoc);
  kfree (aclsoc->addr_map);
  
  kfree (aclsoc);
  aclsoc = 0;
}


/* Initialize the driver module (but not any device) and register
 * the module with the kernel PCI subsystem. */
static int __init aclsoc_init(void) {

  ACL_DEBUG (KERN_DEBUG "----------------------------");
  ACL_DEBUG (KERN_DEBUG "Driver version: %s", ACL_DRIVER_VERSION);
  aclsoc_probe();
  return 0; // platform_driver_register (&aclsoc_driver);
}

static void __exit aclsoc_exit(void)
{
//  platform_driver_unregister(&aclsoc_driver);
  aclsoc_remove();
  printk(KERN_DEBUG "aclsoc driver is unloaded!\n");
}


module_init (aclsoc_init);
module_exit (aclsoc_exit);

