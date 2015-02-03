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


/* Handling of special commands (anything that is not read/write/open/close)
 * that user may call.
 * See pcie_linux_driver_exports.h for explanations of each command. */


#include <linux/mm.h>
#include <linux/device.h>
#include <linux/sched.h>

#include "aclsoc.h"

int reprogram_via_fpga_manager (struct aclsoc_dev *aclsoc, __user char* rbf_image, size_t rbf_image_size);


/* Execute special command */
ssize_t aclsoc_exec_cmd (struct aclsoc_dev *aclsoc, 
                         struct acl_cmd kcmd, 
                         size_t count) {
  ssize_t result = 0;

  switch (kcmd.command) {  
  case ACLPCI_CMD_GET_DMA_IDLE_STATUS: {
    u32 idle = 1; // no DMA for now
    result = copy_to_user ( kcmd.user_addr, &idle, sizeof(idle) );
    break;
  }

  case ACLPCI_CMD_DMA_UPDATE: {
    break;
  }
  
  case ACLPCI_CMD_ENABLE_KERNEL_IRQ: {
    unmask_kernel_irq(aclsoc);
    break;
  }
  
  case ACLPCI_CMD_SET_SIGNAL_PAYLOAD: {
    u32 id;
    result = copy_from_user ( &id, kcmd.user_addr, sizeof(id) );
    aclsoc->signal_info.si_int = id;
    break;
  }
  
  case ACLPCI_CMD_GET_DRIVER_VERSION: {
    /* Driver version is a string */
    result = copy_to_user ( kcmd.user_addr, &ACL_DRIVER_VERSION, strlen(ACL_DRIVER_VERSION)+1 );
    break;
  }

  case ACLPCI_CMD_GET_DEVICE_ID: {
    u32 id = ACL_C5DK_DEVICE_ID;
    result = copy_to_user ( kcmd.user_addr, &id, sizeof(id) );
    break;
  }
  
  case ACLPCI_CMD_GET_PHYS_PTR_FROM_VIRT: {
    unsigned long vm_addr;
    int i;
    result = copy_from_user ( &vm_addr, kcmd.user_addr, sizeof(vm_addr) );
    
    for (i = 0; i < 128; i++) {
      if (aclsoc->addr_map[i].vm_start == 0) break;
      if (aclsoc->addr_map[i].vm_start == vm_addr) {
        result = copy_to_user ( kcmd.device_addr, &aclsoc->addr_map[i].dma_handle, sizeof(kcmd.device_addr) );
        break;
      }
    }
    break;
  }

  case ACLPCI_CMD_LOAD_PCI_CONTROL_REGS:
    /* Things to do after reprogramming the FGPA */
    init_irq (aclsoc);
    break;
    
  case ACLPCI_CMD_SAVE_PCI_CONTROL_REGS:
    /* Prepare for reprogramming the FGPA */
    release_irq (aclsoc);
    break;

  case ACLPCI_CMD_UNPIN_USER_ADDR:
  case ACLPCI_CMD_PIN_USER_ADDR:
  case ACLPCI_CMD_GET_VENDOR_ID:
  case ACLPCI_CMD_GET_PCI_GEN: 
  case ACLPCI_CMD_GET_PCI_NUM_LANES:
  case ACLPCI_CMD_DO_CVP:
  case ACLPCI_CMD_REPROGRAM_VIA_FPGA_MANAGER:
  default:
    ACL_DEBUG (KERN_WARNING " Invalid or unsupported command %u! Ignoring the call. See aclsoc_common.h for list of understood commands", kcmd.command);
    result = -EFAULT;
    break;
  }
  
  return result;
}


