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

//
// sysfs path to our device and other properties
//
#define DEMO_DRIVER_DEV_SYSFS_ENTRY_DIR "/sys/devices/sopc.0/c0000000.bridge/ff230000.driver"
#define UIO_NAME "demo_uio"
#define UIO_VERSION "1.0"
#define UIO_MAP_NAME "demo_uio_driver_hw_region"
#define UIO_MAP_ADDR "0xff230000"
#define UIO_MAP_OFFSET "0x0"
#define UIO_MAP_SIZE "0x1000"

//
// buffer allocation size constants
//
#define DIR_NAME_BUFFER_SIZE	(1024)
#define FILE_NAME_BUFFER_SIZE	(1024)
#define RESPONSE_BUFFER_SIZE	(4096)
#define DEV_NAME_BUFFER_SIZE	(1024)

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
Usage: driver_uio_test [ONE-OPTION-ONLY]\n\
  -t, --print-timer\n\
  -o, --dump-rom\n\
  -a, --dump-ram\n\
  -f, --fill-ram\n\
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
  -o, --dump-rom\n\
Dump the binary ROM contents out stdout.\n\
\n\
  -a, --dump-ram\n\
Dump the binary RAM contents out stdout.\n\
\n\
  -f, --fill-ram\n\
Write stdin to the RAM contents.\n\
\n\
  -h, --help\n\
Display this help message.\n\
\n\
"  

