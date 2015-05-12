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

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <error.h>
#include <dirent.h>
#include <string.h>

#include "demo_devmem.h"

//
// globals declared for command line status variables
//
void *g_print_timer	= NULL;
void *g_stop_timer	= NULL;
void *g_dump_rom	= NULL;
void *g_dump_ram	= NULL;
void *g_fill_ram	= NULL;
void *g_dma_rom_ram	= NULL;
void *g_help		= NULL;

//
// prototypes
//
void validate_system_features(void);
void parse_cmdline(int argc, char **argv);
void do_print_timer(void *demo_driver_map);
void do_stop_timer(void *demo_driver_map);
void do_dump_rom(void *demo_driver_map);
void do_dump_ram(void *demo_driver_map);
void do_fill_ram(void *demo_driver_map);
void do_dma_rom_ram(void *memcpy_msgdma_map);
void do_help(void);

//
// main
//
int main(int argc, char **argv) {

	int devmem_fd;
	void *demo_driver_map;
	void *memcpy_msgdma_map;
	int result;
	
	//
	// validate the system features
	//
	validate_system_features();
	
	//
	// verify that DEMO_DRIVER_PHYS_BASE is page aligned
	//
	if(DEMO_DRIVER_PHYS_BASE & (sysconf(_SC_PAGE_SIZE) - 1)) {
		error(0, 0, "%s:%d DEMO_DRIVER_PHYS_BASE is not page aligned", __FILE__, __LINE__);
		error(0, 0, "%s:%d  DEMO_DRIVER_PHYS_BASE = 0x%08X", __FILE__, __LINE__, DEMO_DRIVER_PHYS_BASE);
		error(1, 0, "%s:%d sysconf(_SC_PAGE_SIZE) = 0x%08lX", __FILE__, __LINE__, (sysconf(_SC_PAGE_SIZE) - 1));
	}
	
	//
	// verify that MEMCPY_MSGDMA_CSR_PHYS_BASE is page aligned
	//
	if(MEMCPY_MSGDMA_CSR_PHYS_BASE & (sysconf(_SC_PAGE_SIZE) - 1)) {
		error(0, 0, "%s:%d MEMCPY_MSGDMA_CSR_PHYS_BASE is not page aligned", __FILE__, __LINE__);
		error(0, 0, "%s:%d  MEMCPY_MSGDMA_CSR_PHYS_BASE = 0x%08X", __FILE__, __LINE__, MEMCPY_MSGDMA_CSR_PHYS_BASE);
		error(1, 0, "%s:%d sysconf(_SC_PAGE_SIZE) = 0x%08lX", __FILE__, __LINE__, (sysconf(_SC_PAGE_SIZE) - 1));
	}
	
	//
	// parse the command line arguments
	//
	parse_cmdline(argc, argv);

	//
	// open() the /dev/mem device
	//
	devmem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(devmem_fd < 0) {
		perror("devmem open");
		exit(EXIT_FAILURE);
	}

	//
	// mmap() the base of our demo_driver hardware
	//
	demo_driver_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, DEMO_DRIVER_PHYS_BASE);
	if(demo_driver_map == MAP_FAILED) {
		perror("devmem mmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	//
	// mmap() the base of our memcpy_msgdma hardware
	//
	memcpy_msgdma_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, MEMCPY_MSGDMA_CSR_PHYS_BASE);
	if(memcpy_msgdma_map == MAP_FAILED) {
		perror("devmem mmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	//
	// perform the operation selected by the command line arguments
	//
	if(g_print_timer	!= NULL) do_print_timer(demo_driver_map);
	if(g_stop_timer		!= NULL) do_stop_timer(demo_driver_map);
	if(g_dump_rom		!= NULL) do_dump_rom(demo_driver_map);
	if(g_dump_ram		!= NULL) do_dump_ram(demo_driver_map);
	if(g_fill_ram		!= NULL) do_fill_ram(demo_driver_map);
	if(g_dma_rom_ram	!= NULL) do_dma_rom_ram(memcpy_msgdma_map);
	if(g_help		!= NULL) do_help();

	//
	// munmap everything and close the /dev/mem file descriptor
	//
	result = munmap(demo_driver_map, sysconf(_SC_PAGE_SIZE));
	if(result < 0) {
		perror("devmem munmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	result = munmap(memcpy_msgdma_map, sysconf(_SC_PAGE_SIZE));
	if(result < 0) {
		perror("devmem munmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	close(devmem_fd);
	exit(EXIT_SUCCESS);
}

//
// This function attempts to validate some of the features that we expect to
// see in the system that we are running on.  Primarily this is an attempt to
// validate that the macros that we've defined for  DEMO_DRIVER_PHYS_BASE and
// DEMO_DRIVER_FREQ are accurate.  We do this by checking to see if the
// expected entries in the procfs and sysfs exist for our device.  Then we
// attempt to verify that the clocks setting for our device in the device-tree
// matches the clock that we expect to be driving it.  We then perform similar
// checks against the memcpy_msgdma component to verify it's base addresses.
//
void validate_system_features(void) {
	
	DIR *dp;
	const char *dirname;
	int fd;
	const char *filename;
	int result;
	unsigned char clocks_array[4];
	unsigned char phandle_array[4];
	unsigned char memcpy_msgdma_reg_names[] = MEMCPY_MSGDMA_REG_NAMES_VALUE;
	unsigned char reg_names_in[sizeof(memcpy_msgdma_reg_names)] = { 0 };
	unsigned char memcpy_msgdma_reg[] = MEMCPY_MSGDMA_REG_VALUE;
	unsigned char reg_in[sizeof(memcpy_msgdma_reg)] = { 0 };
	
	//
	// test to see that the demo_driver device entry exists in the sysfs
	//
	dirname = DEMO_DRIVER_SYSFS_ENTRY_DIR;
	dp = opendir(dirname);
	if(dp == NULL) {
		perror("opendir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	if(closedir(dp)) {
		perror("closedir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	//
	// test to see that the demo_driver device entry exists in the procfs
	//
	dirname = DEMO_DRIVER_PROCFS_ENTRY_DIR;
	dp = opendir(dirname);
	if(dp == NULL) {
		perror("opendir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	if(closedir(dp)) {
		perror("closedir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	//
	// fetch the clocks value out of our device entry
	//
	filename = DEMO_DRIVER_CLOCKS_ENTRY;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, clocks_array, 4);
	if(result != 4) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read 4 bytes from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	//
	// fetch the phandle value out of the h2f_user1_clock node
	//
	filename = H2F_USER1_CLOCK_PHANDLE_ENTRY;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, phandle_array, 4);
	if(result != 4) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read 4 bytes from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	//
	// compare the clocks value with the phandle value
	//
	if(memcmp(clocks_array, phandle_array, 4)) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "clocks value does not match phandle value");
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}

	//
	// fetch the reg_names value out of the memcpy_msgdma node
	//
	filename = MEMCPY_MSGDMA_REG_NAMES_ENTRY;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, reg_names_in, sizeof(memcpy_msgdma_reg_names));
	if(result != sizeof(memcpy_msgdma_reg_names)) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read %d bytes from file", sizeof(memcpy_msgdma_reg_names));
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	//
	// compare the reg_names retreived values with our expected values
	//
	if(memcmp(reg_names_in, memcpy_msgdma_reg_names, sizeof(memcpy_msgdma_reg_names))) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "unexpected reg_names value");
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}

	//
	// fetch the reg value out of the memcpy_msgdma node
	//
	filename = MEMCPY_MSGDMA_REG_ENTRY;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, reg_in, sizeof(memcpy_msgdma_reg));
	if(result != sizeof(memcpy_msgdma_reg)) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read %d bytes from file", sizeof(memcpy_msgdma_reg));
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	//
	// compare the reg_names retreived values with our expected values
	//
	if(memcmp(reg_in, memcpy_msgdma_reg, sizeof(memcpy_msgdma_reg))) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "unexpected reg value");
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
}

void parse_cmdline(int argc, char **argv) {

	int c;
	int option_index = 0;
	int bad_input_parsed = 0;
	int action_count = 0;
	static struct option long_options[] = {
		{"print-timer",	no_argument,	NULL, 't'},
		{"stop-timer",	no_argument,	NULL, 's'},
		{"dump-rom", 	no_argument,	NULL, 'o'},
		{"dump-ram", 	no_argument,	NULL, 'a'},
		{"fill-ram", 	no_argument,	NULL, 'f'},
		{"dma-rom-ram",	no_argument,	NULL, 'd'},
		{"help", 	no_argument,	NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	
	//
	// parse the command line arguments
	//
	while(1) {
		c = getopt_long( argc, argv, "tsoafdh", long_options, &option_index);
	
		if(c == -1)
			break;
		switch(c) {
		case 0:
			error(0, 0, "%s:%d getopt_long parsed a value ZERO.",  __FILE__, __LINE__);
			bad_input_parsed = 1;
			break;
		case 't':
			g_print_timer = &g_print_timer;
			break;
		case 's':
			g_stop_timer = &g_stop_timer;
			break;
		case 'o':
			g_dump_rom = &g_dump_rom;
			break;
		case 'a':
			g_dump_ram = &g_dump_ram;
			break;
		case 'f':
			g_fill_ram = &g_fill_ram;
			break;
		case 'd':
			g_dma_rom_ram = &g_dma_rom_ram;
			break;
		case 'h':
			g_help = &g_help;
			break;
		default:
			error(0, 0, "%s:%d getopt_long parsed a value 0x%X", __FILE__, __LINE__, c);
			bad_input_parsed = 1;
			break;
		}
	}
	
	//
	// if we had any parsing errors we just exit here
	//
	if(bad_input_parsed != 0)
		error(1, 0, "%s:%d getopt_long parsed bad input, exiting...", __FILE__, __LINE__);

	//
	// if we had any extra junk on the command line we just exit here
	//
	if(optind < argc) {
		puts("Extra non-option arguments on command line:\n");
		while(optind < argc) {
			printf("%s\n", argv[optind++]);
		}
		error(1, 0, "%s:%d getopt_long parsed extra command line garbage, exiting...", __FILE__, __LINE__);
	}
	
	//
	// verify that we only collected ONE action to perform
	//
	if(g_print_timer	!= NULL) action_count++;
	if(g_stop_timer		!= NULL) action_count++;
	if(g_dump_rom		!= NULL) action_count++;
	if(g_dump_ram		!= NULL) action_count++;
	if(g_fill_ram		!= NULL) action_count++;
	if(g_dma_rom_ram	!= NULL) action_count++;
	if(g_help		!= NULL) action_count++;
	
	if(action_count == 0) {
		puts(USAGE_STR);
		error(1, 0, "%s:%d no options parsed", __FILE__, __LINE__);
	}

	if(action_count > 1) {
		puts(USAGE_STR);
		error(1, 0, "%s:%d too many options parsed", __FILE__, __LINE__);
	}
}

void do_print_timer(void *demo_driver_map) {
	volatile unsigned long *timer_base = (unsigned long *)((unsigned long)demo_driver_map + TIMER_OFST);
	double f_timeout_period = (double)(0.010) / ((double)(1) / (double)(DEMO_DRIVER_FREQ));
	unsigned long timeout_period = f_timeout_period;
	unsigned long timer_snaps_l[4];
	unsigned long timer_snaps_h[4];
	int i;

	//
	// if timer is not running, start it
	//
	if((timer_base[ALTERA_AVALON_TIMER_STATUS_REG] & ALTERA_AVALON_TIMER_STATUS_RUN_MSK) == 0) {
		printf("Timer not currently running, initializing and starting timer.\n");
		timeout_period--;
		timer_base[ALTERA_AVALON_TIMER_PERIODL_REG] = timeout_period;
		timer_base[ALTERA_AVALON_TIMER_PERIODH_REG] = timeout_period >> 16;
		timer_base[ALTERA_AVALON_TIMER_CONTROL_REG] = ALTERA_AVALON_TIMER_CONTROL_CONT_MSK | ALTERA_AVALON_TIMER_CONTROL_START_MSK;
	}
	
	//
	// Dump the timer registers
	//
	printf("  status = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_STATUS_REG]);
	printf(" control = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_CONTROL_REG]);
	printf("period_l = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_PERIODL_REG]);
	printf("period_h = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_PERIODH_REG]);
	printf("  snap_l = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_SNAPL_REG]);
	printf("  snap_h = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_SNAPH_REG]);

	//
	// read the timer snapshot register 4 consecutive times
	//
	timer_base[ALTERA_AVALON_TIMER_SNAPL_REG] = 0x00;
	timer_snaps_l[0] = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
	timer_snaps_h[0] = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];
	
	timer_base[ALTERA_AVALON_TIMER_SNAPL_REG] = 0x00;
	timer_snaps_l[1] = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
	timer_snaps_h[1] = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];

	timer_base[ALTERA_AVALON_TIMER_SNAPL_REG] = 0x00;
	timer_snaps_l[2] = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
	timer_snaps_h[2] = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];

	timer_base[ALTERA_AVALON_TIMER_SNAPL_REG] = 0x00;
	timer_snaps_l[3] = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
	timer_snaps_h[3] = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];
	
	//
	// catenate the 16-bit snapshot values into their 32-bit values
	//
	timer_snaps_l[0] = (timer_snaps_h[0] << 16) | (timer_snaps_l[0] & 0xFFFF);
	timer_snaps_l[1] = (timer_snaps_h[1] << 16) | (timer_snaps_l[1] & 0xFFFF);
	timer_snaps_l[2] = (timer_snaps_h[2] << 16) | (timer_snaps_l[2] & 0xFFFF);
	timer_snaps_l[3] = (timer_snaps_h[3] << 16) | (timer_snaps_l[3] & 0xFFFF);
	
	//
	// print the snapshot statistics
	//
	for(i = 0 ; i < 4 ; i++) {
		printf("timer snapshot[%d] = 0x%08lX\n", i, timer_snaps_l[i]);
	}
	
	for(i = 0 ; i < 3 ; i++) {
		printf("difference between snapshots [%d] and [%d] = 0x%08lX\n",
			i, 
			i + 1,
			(timer_snaps_l[i] > timer_snaps_l[i + 1]) ?
				(timer_snaps_l[i] - timer_snaps_l[i + 1]) :
				(timer_snaps_l[i + 1] - timer_snaps_l[i])
		);
	}
}

void do_stop_timer(void *demo_driver_map) {
	volatile unsigned long *timer_base = (unsigned long *)((unsigned long)demo_driver_map + TIMER_OFST);

	//
	// if timer is running, stop it
	//
	if((timer_base[ALTERA_AVALON_TIMER_STATUS_REG] & ALTERA_AVALON_TIMER_STATUS_RUN_MSK) == 0) {
		printf("Timer not currently running.\n");
	} else {
		printf("Stopping timer.\n");
		timer_base[ALTERA_AVALON_TIMER_CONTROL_REG] = ALTERA_AVALON_TIMER_CONTROL_STOP_MSK;
		timer_base[ALTERA_AVALON_TIMER_STATUS_REG] = 0;
	}
	
}

void do_dump_rom(void *demo_driver_map) {
	write(STDOUT_FILENO, (void *)((unsigned long)demo_driver_map + ROM_OFST), ROM_SPAN);
}

void do_dump_ram(void *demo_driver_map) {
	write(STDOUT_FILENO, (void *)((unsigned long)demo_driver_map + RAM_OFST), RAM_SPAN);
}

void do_fill_ram(void *demo_driver_map) {
	read(STDIN_FILENO, (void *)((unsigned long)demo_driver_map + RAM_OFST), RAM_SPAN);
}

void do_dma_rom_ram(void *memcpy_msgdma_map) {
	volatile unsigned long *memcpy_msgdma_csr_base = (unsigned long *)memcpy_msgdma_map;
	volatile unsigned long *memcpy_msgdma_desc_base = (unsigned long *)((unsigned long)memcpy_msgdma_map + MEMCPY_MSGDMA_DESC_PHYS_OFST);
	unsigned long temp;
	
	//
	// make sure the DMA is not busy and the descriptor buffer is empty
	//
	temp = memcpy_msgdma_csr_base[ALTERA_MSGDMA_CSR_STATUS_REG];
	if((temp & ALTERA_MSGDMA_CSR_BUSY_MASK) != 0x00) {
		error(0, 0, "dma is busy before first use");
		exit(EXIT_FAILURE);
	}
	if((temp & ALTERA_MSGDMA_CSR_DESCRIPTOR_BUFFER_EMPTY_MASK) == 0x00) {
		error(0, 0, "dma descriptor buffer is not empty before first use");
		exit(EXIT_FAILURE);
	}

	//
	// clear the DMA control register
	//
	memcpy_msgdma_csr_base[ALTERA_MSGDMA_CSR_CONTROL_REG] = 0x00;
	
	//
	// start the DMA
	//
	memcpy_msgdma_desc_base[ALTERA_MSGDMA_DESCRIPTOR_READ_ADDRESS_REG] =
			DEMO_DRIVER_PHYS_BASE + ROM_OFST;
	memcpy_msgdma_desc_base[ALTERA_MSGDMA_DESCRIPTOR_WRITE_ADDRESS_REG] =
			DEMO_DRIVER_PHYS_BASE + RAM_OFST;
	memcpy_msgdma_desc_base[ALTERA_MSGDMA_DESCRIPTOR_LENGTH_REG] =
			ROM_SPAN;
	memcpy_msgdma_desc_base[ALTERA_MSGDMA_DESCRIPTOR_CONTROL_STANDARD_REG] =
			ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK;

	//
	// wait for DMA to complete
	//
	do {
		temp = memcpy_msgdma_csr_base[ALTERA_MSGDMA_CSR_STATUS_REG];
	} while((temp & ALTERA_MSGDMA_CSR_BUSY_MASK) != 0x00);
}

void do_help(void) {
	puts(HELP_STR);
	puts(USAGE_STR);
}

