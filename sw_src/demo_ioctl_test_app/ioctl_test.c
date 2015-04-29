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
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ioctl_test.h"

//
// globals declared for command line status variables
//
void *g_get_interval	= NULL;
void *g_set_interval	= NULL;
void *g_get_max_delay	= NULL;
void *g_get_min_delay	= NULL;
void *g_help		= NULL;

unsigned long g_new_interval = 0;

//
// prototypes
//
void parse_cmdline(int argc, char **argv);
void do_help(void);
void do_get_interval(int dev_demo_tmr_fd);
void do_set_interval(int dev_demo_tmr_fd);
void do_get_max_delay(int dev_demo_tmr_fd);
void do_get_min_delay(int dev_demo_tmr_fd);

//
// main
//
int main(int argc, char **argv) {

	int dev_demo_tmr_fd;
	
	//
	// parse the command line arguments
	//
	parse_cmdline(argc, argv);

	//
	// open() the /dev/demo_tmr device
	//
	dev_demo_tmr_fd = open("/dev/demo_tmr", O_RDWR | O_SYNC);
	if(dev_demo_tmr_fd < 0) {
		perror("dev_demo_tmr open");
		exit(EXIT_FAILURE);
	}

	//
	// perform the operation selected by the command line arguments
	//
	if(g_get_interval	!= NULL) do_get_interval(dev_demo_tmr_fd);
	if(g_set_interval	!= NULL) do_set_interval(dev_demo_tmr_fd);
	if(g_get_max_delay	!= NULL) do_get_max_delay(dev_demo_tmr_fd);
	if(g_get_min_delay	!= NULL) do_get_min_delay(dev_demo_tmr_fd);
	if(g_help		!= NULL) do_help();

	close(dev_demo_tmr_fd);
	exit(EXIT_SUCCESS);
}

void parse_cmdline(int argc, char **argv) {

	int c;
	int option_index = 0;
	int bad_input_parsed = 0;
	int action_count = 0;
	char *endptr = NULL;
	static struct option long_options[] = {
		{"get-interval",	no_argument,		NULL, 'g'},
		{"set-interval",	required_argument,	NULL, 's'},
		{"get-max-delay",	no_argument,		NULL, 'x'},
		{"get-min-delay",	no_argument,		NULL, 'n'},
		{"help",		no_argument,		NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	
	//
	// parse the command line arguments
	//
	while(1) {
		c = getopt_long( argc, argv, "gs:xnh", long_options, &option_index);
	
		if(c == -1)
			break;
		switch(c) {
		case 0:
			error(0, 0, "%s:%d getopt_long parsed a value ZERO.",  __FILE__, __LINE__);
			bad_input_parsed = 1;
			break;
		case 'g':
			g_get_interval = &g_get_interval;
			break;
		case 's':
			g_set_interval = &g_set_interval;
			if(*optarg == '\0') {
				bad_input_parsed = 1;
				break;
			}
			g_new_interval = strtoul(optarg, &endptr, 0);
			if(*endptr != '\0') {
				bad_input_parsed = 1;
				break;
			}
			break;
		case 'x':
			g_get_max_delay = &g_get_max_delay;
			break;
		case 'n':
			g_get_min_delay = &g_get_min_delay;
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
	if(g_get_interval	!= NULL) action_count++;
	if(g_set_interval	!= NULL) action_count++;
	if(g_get_max_delay	!= NULL) action_count++;
	if(g_get_min_delay	!= NULL) action_count++;
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

void do_help(void) {
	puts(HELP_STR);
	puts(USAGE_STR);
}

void do_get_interval(int dev_demo_tmr_fd) {

	int result;
	unsigned long interval;

	result = ioctl(dev_demo_tmr_fd, IOC_GET_INTERVAL, &interval);
	if(result != 0) {
		error(1, errno, "%s:%d ioctl failed", __FILE__, __LINE__);
	}
	
	printf("Current IRQ interval is %lu interrupts per second.\n", interval);
}

void do_set_interval(int dev_demo_tmr_fd) {

	int result;

	result = ioctl(dev_demo_tmr_fd, IOC_SET_INTERVAL, &g_new_interval);
	if(result != 0) {
		error(1, errno, "%s:%d ioctl failed", __FILE__, __LINE__);
	}
	
	printf("IRQ interval set to %lu interrupts per second.\n", g_new_interval);
}

void do_get_max_delay(int dev_demo_tmr_fd) {

	int result;
	unsigned long delay;

	result = ioctl(dev_demo_tmr_fd, IOC_GET_MAX_DELAY, &delay);
	if(result != 0) {
		error(1, errno, "%s:%d ioctl failed", __FILE__, __LINE__);
	}
	
	printf("Maximum IRQ service delay is %lu ticks.\n", delay);
}

void do_get_min_delay(int dev_demo_tmr_fd) {

	int result;
	unsigned long delay;

	result = ioctl(dev_demo_tmr_fd, IOC_GET_MIN_DELAY, &delay);
	if(result != 0) {
		error(1, errno, "%s:%d ioctl failed", __FILE__, __LINE__);
	}
	
	printf("Minimum IRQ service delay is %lu ticks.\n", delay);
}

