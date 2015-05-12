/*
 * Copyright (c) 2014, Altera Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "my_altera_avalon_timer_regs.h"
#include "my_altera_msgdma_csr_regs.h"
#include "my_altera_msgdma_descriptor_regs.h"

//
// expected values for the physical base address and clock frequency of the
// demo driver hardware
//
#define DEMO_DRIVER_PHYS_BASE	(0xFF230000)
#define DEMO_DRIVER_SYSFS_ENTRY_DIR "/sys/bus/platform/devices/ff230000.driver"
#define DEMO_DRIVER_PROCFS_ENTRY_DIR "/proc/device-tree/sopc@0/bridge@0xc0000000/driver@0x100030000"
#define DEMO_DRIVER_FREQ	(50000000)
#define DEMO_DRIVER_CLOCKS_ENTRY "/proc/device-tree/sopc@0/bridge@0xc0000000/driver@0x100030000/clocks"
#define H2F_USER1_CLOCK_PHANDLE_ENTRY "/proc/device-tree/clocks/clk_0/linux,phandle"
//
// expected values for the physical base addresses of the memcpy_msgdma core
//
#define MEMCPY_MSGDMA_CSR_PHYS_BASE (0xFF220000)
#define MEMCPY_MSGDMA_DESC_PHYS_BASE (0xFF220020)
#define MEMCPY_MSGDMA_DESC_PHYS_OFST (MEMCPY_MSGDMA_DESC_PHYS_BASE - MEMCPY_MSGDMA_CSR_PHYS_BASE)
#define MEMCPY_MSGDMA_REG_NAMES_ENTRY "/proc/device-tree/sopc@0/bridge@0xc0000000/msgdma@0x100020000/reg-names"
#define MEMCPY_MSGDMA_REG_NAMES_VALUE { 0x63, 0x73, 0x72, 0x00, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x6f, 0x72, 0x5f, 0x73, 0x6c, 0x61, 0x76, 0x65, 0x00 }
#define MEMCPY_MSGDMA_REG_ENTRY "/proc/device-tree/sopc@0/bridge@0xc0000000/msgdma@0x100020000/reg"
#define MEMCPY_MSGDMA_REG_VALUE { 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10 }

//
// demo driver hardware map
//
#define ROM_OFST	(0)
#define ROM_SPAN	(1024)
#define RAM_OFST	(ROM_OFST + ROM_SPAN)
#define RAM_SPAN	(1024)
#define TIMER_OFST	(RAM_OFST + RAM_SPAN)

//
// usage string
//
#define USAGE_STR "\
\n\
Usage: driver_devmem [ONE-OPTION-ONLY]\n\
  -t, --print-timer\n\
  -s, --stop-timer\n\
  -o, --dump-rom\n\
  -a, --dump-ram\n\
  -f, --fill-ram\n\
  -d, --dma-rom-ram\n\
  -h, --help\n\
\n\
"  

//
// help string
//
#define HELP_STR "\
\n\
Only one of the following options may be passed in per invocation:\n\
\n\
  -t, --print-timer\n\
Print the timer statistics out stdout.\n\
\n\
  -s, --stop-timer\n\
Stop the timer.\n\
\n\
  -o, --dump-rom\n\
Dump the binary ROM contents out stdout.\n\
\n\
  -a, --dump-ram\n\
Dump the binary RAM contents out stdout.\n\
\n\
  -f, --fill-ram\n\
Write stdin to the RAM contents.\n\
\n\
  -d, --dma-rom-ram\n\
dma the ROM contents to the RAM contents.\n\
\n\
  -h, --help\n\
Display this help message.\n\
\n\
"  

