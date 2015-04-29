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

#include "demo_map_test.h"

//
// globals declared for command line status variables
//
void *g_print_timer	= NULL;
void *g_dump_rom	= NULL;
void *g_dump_ram	= NULL;
void *g_fill_ram	= NULL;
void *g_help		= NULL;

//
// prototypes
//
void parse_cmdline(int argc, char **argv);
void do_print_timer(void *demo_driver_map);
void do_dump_rom(void *demo_driver_map);
void do_dump_ram(void *demo_driver_map);
void do_fill_ram(void *demo_driver_map);
void do_help(void);

//
// main
//
int main(int argc, char **argv) {

	int dev_demo_map_fd;
	void *demo_driver_map;
	int result;
	
	//
	// parse the command line arguments
	//
	parse_cmdline(argc, argv);

	//
	// open() the /dev/mem device
	//
	dev_demo_map_fd = open("/dev/demo_map", O_RDWR | O_SYNC);
	if(dev_demo_map_fd < 0) {
		perror("dev_demo_map open");
		exit(EXIT_FAILURE);
	}

	//
	// mmap() the base of our demo_driver hardware
	//
	demo_driver_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, dev_demo_map_fd, 0);
	if(demo_driver_map == MAP_FAILED) {
		perror("dev_demo_map mmap");
		close(dev_demo_map_fd);
		exit(EXIT_FAILURE);
	}

	//
	// perform the operation selected by the command line arguments
	//
	if(g_print_timer	!= NULL) do_print_timer(demo_driver_map);
	if(g_dump_rom		!= NULL) do_dump_rom(demo_driver_map);
	if(g_dump_ram		!= NULL) do_dump_ram(demo_driver_map);
	if(g_fill_ram		!= NULL) do_fill_ram(demo_driver_map);
	if(g_help		!= NULL) do_help();

	//
	// munmap everything and close the /dev/mem file descriptor
	//
	result = munmap(demo_driver_map, sysconf(_SC_PAGE_SIZE));
	if(result < 0) {
		perror("dev_demo_map munmap");
		close(dev_demo_map_fd);
		exit(EXIT_FAILURE);
	}

	close(dev_demo_map_fd);
	exit(EXIT_SUCCESS);
}

void parse_cmdline(int argc, char **argv) {

	int c;
	int option_index = 0;
	int bad_input_parsed = 0;
	int action_count = 0;
	static struct option long_options[] = {
		{"print-timer",	no_argument,	NULL, 't'},
		{"dump-rom", 	no_argument,	NULL, 'o'},
		{"dump-ram", 	no_argument,	NULL, 'a'},
		{"fill-ram", 	no_argument,	NULL, 'f'},
		{"help", 	no_argument,	NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	
	//
	// parse the command line arguments
	//
	while(1) {
		c = getopt_long( argc, argv, "toafh", long_options, &option_index);
	
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
		case 'o':
			g_dump_rom = &g_dump_rom;
			break;
		case 'a':
			g_dump_ram = &g_dump_ram;
			break;
		case 'f':
			g_fill_ram = &g_fill_ram;
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
	if(g_dump_rom		!= NULL) action_count++;
	if(g_dump_ram		!= NULL) action_count++;
	if(g_fill_ram		!= NULL) action_count++;
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

	//
	// Dump the timer registers
	//
	printf("NOTE: current register values are only read from the hardware, so as not to\n");
	printf("      contend with the IRQ handler or other driver activities that may occur\n");
	printf("      simultaneously, as we cannot lock our accesses from user space\n");
	printf("  status = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_STATUS_REG]);
	printf(" control = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_CONTROL_REG]);
	printf("period_l = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_PERIODL_REG]);
	printf("period_h = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_PERIODH_REG]);
	printf("  snap_l = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_SNAPL_REG]);
	printf("  snap_h = 0x%08lX\n", timer_base[ALTERA_AVALON_TIMER_SNAPH_REG]);
}

void do_dump_rom(void *demo_driver_map) {
	write(STDOUT_FILENO, (unsigned long *)((unsigned long)demo_driver_map + ROM_OFST), ROM_SPAN);
}

void do_dump_ram(void *demo_driver_map) {
	write(STDOUT_FILENO, (unsigned long *)((unsigned long)demo_driver_map + RAM_OFST), RAM_SPAN);
}

void do_fill_ram(void *demo_driver_map) {
	read(STDIN_FILENO, (unsigned long *)((unsigned long)demo_driver_map + RAM_OFST), RAM_SPAN);
}

void do_help(void) {
	puts(HELP_STR);
	puts(USAGE_STR);
}

