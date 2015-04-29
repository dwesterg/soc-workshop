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

#include "demo_uio_test.h"

//
// globals declared for command line status variables
//
void *g_run_timer	= NULL;
void *g_dump_rom	= NULL;
void *g_dump_ram	= NULL;
void *g_fill_ram	= NULL;
void *g_help		= NULL;

char g_uio_dev_name[NAME_MAX] = {0};

//
// prototypes
//
void validate_system_features(void);
void parse_cmdline(int argc, char **argv);
void do_run_timer(void *demo_driver_map, int uio_fd);
void do_dump_rom(void *demo_driver_map);
void do_dump_ram(void *demo_driver_map);
void do_fill_ram(void *demo_driver_map);
void do_dma_rom_ram(void *memcpy_msgdma_map);
void do_help(void);

//
// main
//
int main(int argc, char **argv) {

	int devuio_fd;
	void *demo_driver_map;
	int result;
	char dev_name[DEV_NAME_BUFFER_SIZE];
	
	//
	// validate the system features
	//
	validate_system_features();
		
	//
	// parse the command line arguments
	//
	parse_cmdline(argc, argv);
	
	//
	// open() the /dev/uioX device
	//
	strncpy(dev_name, "/dev/", DEV_NAME_BUFFER_SIZE);
	strncat(dev_name, g_uio_dev_name, DEV_NAME_BUFFER_SIZE);
	devuio_fd = open(dev_name, O_RDWR | O_SYNC);
	if(devuio_fd < 0) {
		perror("devuio open");
		exit(EXIT_FAILURE);
	}

	//
	// mmap() the base of our demo_driver hardware
	//
	demo_driver_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, devuio_fd, 0);
	if(demo_driver_map == MAP_FAILED) {
		perror("devuio mmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	//
	// perform the operation selected by the command line arguments
	//
	if(g_run_timer		!= NULL) do_run_timer(demo_driver_map, devuio_fd);
	if(g_dump_rom		!= NULL) do_dump_rom(demo_driver_map);
	if(g_dump_ram		!= NULL) do_dump_ram(demo_driver_map);
	if(g_fill_ram		!= NULL) do_fill_ram(demo_driver_map);
	if(g_help		!= NULL) do_help();

	//
	// munmap everything and close the /dev/mem file descriptor
	//
	result = munmap(demo_driver_map, sysconf(_SC_PAGE_SIZE));
	if(result < 0) {
		perror("devuio munmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	close(devuio_fd);

	exit(EXIT_SUCCESS);
}

//
// This function attempts to validate that our device is currently under the
// control of our uio driver.
//
void validate_system_features(void) {
	
	DIR *dp;
	const char *dirname;
	int fd;
	char *filename;
	struct dirent *dir_entry;
	int found_uio_entry;
	int result;
	char dir_name_buffer[DIR_NAME_BUFFER_SIZE];
	char file_name_buffer[FILE_NAME_BUFFER_SIZE];
	char response_buffer[RESPONSE_BUFFER_SIZE];
	char *newline;
	
	//
	// test to see that the demo_driver device entry exists in the sysfs
	//
	dirname = DEMO_DRIVER_DEV_SYSFS_ENTRY_DIR;
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
	// look for a uio entry
	//
	strncpy(dir_name_buffer, DEMO_DRIVER_DEV_SYSFS_ENTRY_DIR, DIR_NAME_BUFFER_SIZE);
	strncat(dir_name_buffer, "/uio", DIR_NAME_BUFFER_SIZE);
	dirname = dir_name_buffer;
	dp = opendir(dirname);
	if(dp == NULL) {
		perror("opendir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	found_uio_entry = 0;
	do {
		dir_entry = readdir(dp);
		if(dir_entry != NULL) {
			if(dir_entry->d_type == DT_DIR) {
				if(strncmp(dir_entry->d_name, "uio", 3) == 0) {
					strncpy(g_uio_dev_name, dir_entry->d_name, NAME_MAX);
					strncat(dir_name_buffer, "/", DIR_NAME_BUFFER_SIZE);
					strncat(dir_name_buffer, dir_entry->d_name, DIR_NAME_BUFFER_SIZE);
					found_uio_entry = 1;
					break;
				}
			}
		}
	} while(dir_entry != NULL);
	if(closedir(dp)) {
		perror("closedir");
		error(0, 0, "Directory: \'%s\'", dirname);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}

	if(found_uio_entry == 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "unable to locate uio entry for device");
		exit(EXIT_FAILURE);
	}
	
	//
	// test the uio device name
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/name", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_NAME) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio name does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_NAME);
		exit(EXIT_FAILURE);
	}

	//
	// test the uio device version
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/version", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_VERSION) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio version does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_VERSION);
		exit(EXIT_FAILURE);
	}

	//
	// test the uio device maps name
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/maps/map0/name", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_MAP_NAME) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio map name does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_MAP_NAME);
		exit(EXIT_FAILURE);
	}

	//
	// test the uio device maps addr
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/maps/map0/addr", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_MAP_ADDR) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio map addr does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_MAP_ADDR);
		exit(EXIT_FAILURE);
	}

	//
	// test the uio device maps offset
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/maps/map0/offset", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_MAP_OFFSET) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio map offset does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_MAP_OFFSET);
		exit(EXIT_FAILURE);
	}

	//
	// test the uio device maps size
	//
	strncpy(file_name_buffer, dir_name_buffer, FILE_NAME_BUFFER_SIZE);
	strncat(file_name_buffer, "/maps/map0/size", FILE_NAME_BUFFER_SIZE);	
	filename = file_name_buffer;
	fd = open(filename, O_RDONLY);
	if(fd < 0) {
		perror("open");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "system layout does not match expected layout");
		exit(EXIT_FAILURE);
	}
	result = read(fd, response_buffer, RESPONSE_BUFFER_SIZE);
	if(result < 0) {
		perror("read");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "failed to read from file");
		close(fd);
		exit(EXIT_FAILURE);
	}
	if(close(fd)) {
		perror("close");
		error(0, 0, "File: \'%s\'", filename);
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	newline = strchr(response_buffer, '\n');
	if(newline != NULL)
		*newline = '\0';
	if(strcmp(response_buffer, UIO_MAP_SIZE) != 0) {
		error(0, 0, "%s:%d", __FILE__, __LINE__);
		error(0, 0, "uio map size does not match expected");
		error(0, 0, "     got: '%s'", response_buffer);
		error(0, 0, "expected: '%s'", UIO_MAP_SIZE);
		exit(EXIT_FAILURE);
	}
}

void parse_cmdline(int argc, char **argv) {

	int c;
	int option_index = 0;
	int bad_input_parsed = 0;
	int action_count = 0;
	static struct option long_options[] = {
		{"run-timer",	no_argument,	NULL, 't'},
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
			g_run_timer = &g_run_timer;
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
	if(g_run_timer		!= NULL) action_count++;
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

void do_run_timer(void *demo_driver_map, int uio_fd) {
	volatile unsigned long *timer_base = (unsigned long *)((unsigned long)demo_driver_map + TIMER_OFST);
	unsigned long timeout_period;
	unsigned long timer_snapshot;
	unsigned long timer_snaps_l;
	unsigned long timer_snaps_h;
	unsigned long timer_period_l;
	unsigned long timer_period_h;
	unsigned long max_irq_delay;
	unsigned long min_irq_delay;
	unsigned long max_user_delay;
	unsigned long min_user_delay;
	unsigned long irq_control;
	unsigned long irq_count;
	unsigned long next_irq_count;
	int i;

	//
	// verify timer is running
	//
	if((timer_base[ALTERA_AVALON_TIMER_STATUS_REG] & ALTERA_AVALON_TIMER_STATUS_RUN_MSK) == 0) {
		error(1, 0, "%s:%d timer not running after uio dev open", __FILE__, __LINE__);
	}

	//
	// stop the timer irq
	//
	irq_control = 0;
	write(uio_fd, &irq_control, sizeof(irq_control));

	//
	// verify timer is stopped
	//
	if((timer_base[ALTERA_AVALON_TIMER_STATUS_REG] & ALTERA_AVALON_TIMER_STATUS_RUN_MSK) != 0) {
		error(1, 0, "%s:%d timer is running after irq_control write issued to stop", __FILE__, __LINE__);
	}

	//
	// read the period registers, this value should equal 1 second timeout
	//
	timer_period_l = timer_base[ALTERA_AVALON_TIMER_PERIODL_REG];
	timer_period_h = timer_base[ALTERA_AVALON_TIMER_PERIODH_REG];
	
	//
	// set the timeout period to 100 times per second or 10 milliseconds
	//
	timeout_period = (timer_period_l & 0xFFFF) | ((timer_period_h << 16) & 0xFFFF0000);
	timeout_period++;
	timeout_period /= 100;
	timeout_period--;
	timer_base[ALTERA_AVALON_TIMER_PERIODL_REG] = timeout_period;
	timer_base[ALTERA_AVALON_TIMER_PERIODH_REG] = timeout_period >> 16;

	//
	// start the timer irq
	//
	irq_control = 1;
	write(uio_fd, &irq_control, sizeof(irq_control));

	//
	// capture the first 100 interrupts
	//
	max_irq_delay = 0;
	min_irq_delay = 0xFFFFFFFF;
	max_user_delay = 0;
	min_user_delay = 0xFFFFFFFF;

	for(i = 0 ; i < 100 ; i++) {
		//
		// wait for an IRQ event
		//
		read(uio_fd, &irq_count, 4);

		//
		// read the IRQ handler delay
		//
		timer_snaps_l = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
		timer_snaps_h = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];
		
		//
		// snapshot the timer
		//
		timer_base[ALTERA_AVALON_TIMER_SNAPL_REG] = 0;
		
		//
		// verify that we haven't missed an IRQ event
		//
		if(i == 0) {
			next_irq_count = irq_count + 1;
		} else {
			if(next_irq_count != irq_count) {
				error(0, 0, "irq count returned %lu", irq_count);
				error(0, 0, "expected %d", i);
				error(1, 0, "%s:%d missed an IRQ event", __FILE__, __LINE__);
			} else {
				next_irq_count++;
			}
		}
		
		//
		// calculate the IRQ delay statistics
		//
		timer_snapshot = (timer_snaps_l & 0xFFFF) | ((timer_snaps_h << 16) & 0xFFFF0000);
		timer_snapshot = timeout_period - timer_snapshot;
		if(timer_snapshot > max_irq_delay)
			max_irq_delay = timer_snapshot;
		if(timer_snapshot < min_irq_delay)
			min_irq_delay = timer_snapshot;

		//
		// read the user handler delay
		//
		timer_snaps_l = timer_base[ALTERA_AVALON_TIMER_SNAPL_REG];
		timer_snaps_h = timer_base[ALTERA_AVALON_TIMER_SNAPH_REG];
		
		//
		// calculate the user delay statistics
		//
		timer_snapshot = (timer_snaps_l & 0xFFFF) | ((timer_snaps_h << 16) & 0xFFFF0000);
		timer_snapshot = timeout_period - timer_snapshot;
		if(timer_snapshot > max_user_delay)
			max_user_delay = timer_snapshot;
		if(timer_snapshot < min_user_delay)
			min_user_delay = timer_snapshot;
	}
	
	printf("Successfully captured the first 100 timer interrupts.\n");
	printf(" max_irq_delay = 0x%08lX : %lu\n", max_irq_delay, max_irq_delay);
	printf(" min_irq_delay = 0x%08lX : %lu\n", min_irq_delay, min_irq_delay);
	printf("max_user_delay = 0x%08lX : %lu\n", max_user_delay, max_user_delay);
	printf("min_user_delay = 0x%08lX : %lu\n", min_user_delay, min_user_delay);
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

