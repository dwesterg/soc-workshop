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

/* Independent tester of Altera OpenCL CV SoC board + driver without
 * the host run-time library. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "pcie_linux_driver_exports.h"

#define DEEP_DEBUG(x)

unsigned char read_uchar (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned char val;
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val) };
  read (dev_id, &read_cmd, sizeof(read_cmd));
  return val;
}

unsigned short read_ushort (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned short val;
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  read (dev_id, &read_cmd, sizeof(read_cmd));
  return val;
}

unsigned int read_uint (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned int val;
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  read (dev_id, &read_cmd, sizeof(read_cmd));
  DEEP_DEBUG(printf ("-- Read 32bits %x from bar %d, addr %p\n", val, bar_id, dev_addr)); 
  return val;
}

unsigned long read_ulong (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned long val;
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  read (dev_id, &read_cmd, sizeof(read_cmd));
  DEEP_DEBUG(printf ("-- Read 64bits %lx from bar %d, addr %p\n", val, bar_id, dev_addr)); 
  return val;
}

size_t read_mem (ssize_t dev_id, int bar_id, 
                 void *dev_addr, void *dst_addr, size_t count) {

  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, dst_addr, count };
  size_t ret = read (dev_id, &read_cmd, sizeof(read_cmd));
  return ret;
}


void write_uchar (ssize_t dev_id, int bar_id, void *dev_addr, unsigned char val) {
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  write (dev_id, &read_cmd, sizeof(read_cmd));
}

void write_ushort (ssize_t dev_id, int bar_id, void *dev_addr, unsigned short val) {
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  write (dev_id, &read_cmd, sizeof(read_cmd));
}

void write_uint (ssize_t dev_id, int bar_id, void *dev_addr, unsigned int val) {
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  write (dev_id, &read_cmd, sizeof(read_cmd));
  DEEP_DEBUG(printf ("-- Wrote 32bits %x to bar %d, addr %p\n", val, bar_id, dev_addr));
}

void write_ulong (ssize_t dev_id, int bar_id, void *dev_addr, unsigned long val) {
  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val, sizeof(val)  };
  write (dev_id, &read_cmd, sizeof(read_cmd));
  DEEP_DEBUG(printf ("-- Wrote 64bits %lx to bar %d, addr %p\n", val, bar_id, dev_addr));
}

size_t write_mem (ssize_t dev_id, int bar_id, 
                  void *dev_addr, void *src_addr, size_t count) {

  struct acl_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, src_addr, count };
  size_t ret = write (dev_id, &read_cmd, sizeof(read_cmd));
  return ret;
}

/* Test writing single values of various types to board's global memory.
 * The test passes if we read back what we wrote. */
void test_small_writes  (ssize_t f) {

  unsigned char     uc;
  unsigned short    us;
  unsigned int      ui;
  unsigned int   i, ua[10], ub[10];
  int match;

  void *dev_addr = (void*)0x10000; 
  
  write_uchar     (f, 0, dev_addr, 19);
  uc = read_uchar (f, 0, dev_addr);
  fprintf (stderr, "Wrote 19, read back %u\n", uc);
  assert (uc == 19);
  
  write_ushort    (f, 0, dev_addr, 13);
  us = read_ushort(f, 0, dev_addr);
  assert (us == 13);
  
  write_uint     (f, 0, dev_addr, 18987983);
  ui = read_uint (f, 0, dev_addr);
  assert (ui == 18987983);
  
  for (i = 0; i < 10; i++) {
    ua[i] = i*13;
    ub[i] = 0;
  }
  write_mem (f, 0, dev_addr, ua, 10 * sizeof(int));
  read_mem  (f, 0, dev_addr, ub, 10 * sizeof(int));
  match = 1;
  for (i = 0; i < 10; i++) {
    printf ("ua[%d] = %d, ub[%d] = %d\n", i, ua[i], i, ub[i]);
    match &= (ua[i] == ub[i]);
  }
  assert (match);
  
  printf ("test_small_writes PASSED\n");
}


void test_mmap  (ssize_t f) {
  int i, match;
  int size = 7 * 1024 * 1024;
  char *phys_addr = NULL;
  
  // allocate 'size' bytes of physically contiguous memory.
  // get user virtual address ptr back.
  char *mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
  
  // make sure we can use this memory
  match = 1;
  for (i= 0; i< size; i++) {
    mem[i] = (char)i;
  }
  for (i= 0; i< size; i++) {
    if (mem[i] != (char)i) {
      printf ("  %d: mem[i] = %c, i = %c\n", i, mem[i], (char)i);
      match = 0;
    }
  }
  if (!match) {
    assert (match);
  }
  
  struct acl_cmd read_cmd = { ACLPCI_CMD_BAR, ACLPCI_CMD_GET_PHYS_PTR_FROM_VIRT, &phys_addr, &mem, 0 };
  write (f, &read_cmd, sizeof(read_cmd));
  
  printf ("Mapped virt address %p to physical %p\n", mem,  phys_addr);

  // Now try using the physical address and verify that can read the value back from the
  // virtual address
  ssize_t mem_fd = open ( "/dev/mem", O_RDWR);
  char* ptr = (char*)mmap (0, size, PROT_READ, MAP_SHARED, mem_fd, (size_t)phys_addr);
  printf ("Reading from address %p of /dev/mem: \n", phys_addr);
  match = 1;
  for (i=0; i<size; i++) {
    if ((char)i != ptr[i]) {
      printf ("  mismatch at offset %u: expected: %c, got %c\n", i, i, ptr[i]);
      match = 0;
      break;
    }
  }
  if (match) {
    printf ("All values read from /dev/mem are as expected!\n");
  } else {
    printf ("Failed reading back values from /dev/mem!\n");
  }
  munmap (ptr, size);
  close (mem_fd);
  
  if (!match) {
    assert (match);
  }

// unmaps the virtual address but not the underlying physical one. ouch!  
  munmap (mem, size);
  printf ("test_mmap PASSED\n");
}


void test_page_write  (ssize_t f) {

  int page_size = 4096 * 3;
  int page_size_ints = page_size/sizeof(int);
  unsigned int   i, ua[page_size_ints], ub[page_size_ints];

  void *dev_addr = (void*)0x10000; 
  
  for (i = 0; i < page_size_ints; i++) {
    ua[i] = i*2;
    ub[i] = 0;
  }
  write_mem     (f, 0, dev_addr, ua, page_size_ints * sizeof(int));
  read_mem      (f, 0, dev_addr, ub, page_size_ints * sizeof(int));

  for (i = 0; i < page_size_ints; i++) {
    if (ua[i] != ub[i]) {
      printf ("%d: ua = %d, ub = %d\n", i, ua[i], ub[i]);
    }
  }
  printf ("Done test_page_write with %d bytes\n", page_size);
}

void dump_ddr  (ssize_t f) {

  int addr_start = 0x00000;
  int length = 0x0120;
  int i;
  void *dev_addr = (void*)0x10000; 
  
  // set segment;
  unsigned long long seg =  0x10000;
  write_mem (f, 2, (void*)0xc870, &seg, sizeof(seg));
  
  for (i = addr_start; i < length-addr_start; i++) {
    unsigned int r = read_uint (f, 0, dev_addr + i);
    printf ("%0x ", r);
    if (i % 0x40 == 0) {
      printf ("\n0x%x: ", i);
    }
  }
}  


int main() {

  ssize_t f = open ("/dev/acl", O_RDWR);
  if (f == -1) {
    printf ("Couldn't open the device. Did you load the driver?\n");
    return 0;
  } else {
    printf ("Opened the device: file handle #%zu!\n", f);
  }

  // enable bridges
  system ("echo 1 > /sys/class/fpga-bridge/fpga2hps/enable");
  system ("echo 1 > /sys/class/fpga-bridge/hps2fpga/enable");
  system ("echo 1 > /sys/class/fpga-bridge/lwhps2fpga/enable");

  test_small_writes (f);
  test_page_write (f);
  test_mmap(f);
  printf ("Done testing!\n");
  
  close (f);
  return 0;
}

