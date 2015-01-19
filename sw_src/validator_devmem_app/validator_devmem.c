#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <error.h>
#include <sys/time.h>
#include <errno.h>

#include "validator_devmem.h"

//
// globals declared for command line status variables
//
char *g_sign_message			= NULL;
char *g_auth_message			= NULL;
void *g_dump_random_numbers		= NULL;
void *g_dump_entropy_counters_lo	= NULL;
void *g_dump_entropy_counters_hi	= NULL;
void *g_get_entropy_counters_state	= NULL;
void *g_start_entropy_counters		= NULL;
void *g_stop_entropy_counters		= NULL;
void *g_reset_entropy_counters		= NULL;
void *g_help				= NULL;
char *g_output_file			= NULL;

//
// prototypes
//
void parse_cmdline(int argc, char **argv);
void do_sign_message(void *validator_map);
void do_auth_message(void *validator_map);
void do_dump_random_numbers(void *validator_map);
void do_dump_entropy_counters_lo(void *validator_map);
void do_dump_entropy_counters_hi(void *validator_map);
void do_get_entropy_counters_state(void *validator_map);
void do_start_entropy_counters(void *validator_map);
void do_stop_entropy_counters(void *validator_map);
void do_reset_entropy_counters(void *validator_map);
void do_help(void);

//
//main
//
int main(int argc, char **argv) {

	int devmem_fd;
	void *validator_map;
	int result;
	
	//
	// verify that VALIDATOR_PHYS_BASE is page aligned
	//
	if(VALIDATOR_PHYS_BASE & (sysconf(_SC_PAGE_SIZE) - 1)) {
		char *the_file = __FILE__;
		int the_line = __LINE__;
		error(0, 0, "%s:%d VALIDATOR_PHYS_BASE is not page aligned", the_file, the_line);
		error(0, 0, "%s:%d    VALIDATOR_PHYS_BASE = 0x%08X", the_file, the_line, VALIDATOR_PHYS_BASE);
		error(1, 0, "%s:%d sysconf(_SC_PAGE_SIZE) = 0x%08lX", the_file, the_line, (sysconf(_SC_PAGE_SIZE) - 1));
	}
	
	//
	// parse the command line arguments
	//
	parse_cmdline(argc, argv);

	//
	// first try to open /dev/validator_raw and then mmap() the driver
	// if /dev/validator_raw deos not exist, then we fall back to /dev/mem
	// open() the /dev/mem device and mmap() the base of our validator hardware
	//
	devmem_fd = open("/dev/validator_raw", O_RDWR | O_SYNC);
	if(devmem_fd < 0) {
		int errno_saved = errno;
		perror("/dev/validator_raw open");
		if(errno_saved == EAGAIN) {
			printf("/dev/validator_raw is busy.\nThe device must be opened by another process.\n");
			exit(EXIT_FAILURE);
		}
		printf("Will attempt to use /dev/mem instead.");

		devmem_fd = open("/dev/mem", O_RDWR | O_SYNC);
		if(devmem_fd < 0) {
			perror("devmem open");
			exit(EXIT_FAILURE);
		}
	
		validator_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, VALIDATOR_PHYS_BASE);
		if(validator_map == MAP_FAILED) {
			perror("devmem mmap");
			close(devmem_fd);
			exit(EXIT_FAILURE);
		}
	} else {
		printf("/dev/validator_raw succesfully opened.");

		validator_map = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ|PROT_WRITE, MAP_SHARED, devmem_fd, 0);
		if(validator_map == MAP_FAILED) {
			perror("validator_raw mmap");
			close(devmem_fd);
			exit(EXIT_FAILURE);
		}
	}

	if(g_sign_message		!= NULL) do_sign_message(validator_map);
	if(g_auth_message		!= NULL) do_auth_message(validator_map);
	if(g_dump_random_numbers	!= NULL) do_dump_random_numbers(validator_map);
	if(g_dump_entropy_counters_lo	!= NULL) do_dump_entropy_counters_lo(validator_map);
	if(g_dump_entropy_counters_hi	!= NULL) do_dump_entropy_counters_hi(validator_map);
	if(g_get_entropy_counters_state	!= NULL) do_get_entropy_counters_state(validator_map);
	if(g_start_entropy_counters	!= NULL) do_start_entropy_counters(validator_map);
	if(g_stop_entropy_counters	!= NULL) do_stop_entropy_counters(validator_map);
	if(g_reset_entropy_counters	!= NULL) do_reset_entropy_counters(validator_map);
	if(g_help			!= NULL) do_help();
		
	result = munmap(validator_map, sysconf(_SC_PAGE_SIZE));
	if(result < 0) {
		perror("devmem munmap");
		close(devmem_fd);
		exit(EXIT_FAILURE);
	}

	close(devmem_fd);
	exit(EXIT_SUCCESS);
}

void parse_cmdline(int argc, char **argv) {

	int c;
	int option_index = 0;
	int bad_input_parsed = 0;
	int action_count = 0;
	static struct option long_options[] = {
		{"sign-message", 		required_argument,	NULL, 's'},
		{"auth-message", 		required_argument,	NULL, 'a'},
		{"dump-random-numbers", 	no_argument, 		NULL, 'd'},
		{"dump-entropy-counters-lo", 	no_argument, 		NULL, 'e'},
		{"dump-entropy-counters-hi", 	no_argument, 		NULL, 'E'},
		{"get-entropy-counters-state",	no_argument, 		NULL, 'g'},
		{"start-entropy-counters", 	no_argument, 		NULL, 't'},
		{"stop-entropy-counters", 	no_argument, 		NULL, 'p'},
		{"reset-entropy-counters", 	no_argument, 		NULL, 'r'},
		{"help", 			no_argument, 		NULL, 'h'},
		{"output-file", 		required_argument, 	NULL, 'o'},
		{NULL, 0, NULL, 0}
	};
	
	//
	// parse the command line arguments
	//
	while(1) {
		c = getopt_long( argc, argv, "s:a:deEgtprho:", long_options, &option_index);
	
		if(c == -1)
			break;
		switch(c) {
		case 0:
			error(0, 0, "%s:%d getopt_long parsed a value ZERO.",  __FILE__, __LINE__);
			bad_input_parsed = 1;
			break;
		case 's':
			g_sign_message = optarg;
			break;
		case 'a':
			g_auth_message = optarg;
			break;
		case 'd':
			g_dump_random_numbers = &g_dump_random_numbers;
			break;
		case 'e':
			g_dump_entropy_counters_lo = &g_dump_entropy_counters_lo;
			break;
		case 'E':
			g_dump_entropy_counters_hi = &g_dump_entropy_counters_hi;
			break;
		case 'g':
			g_get_entropy_counters_state = &g_get_entropy_counters_state;
			break;
		case 't':
			g_start_entropy_counters = &g_start_entropy_counters;
			break;
		case 'p':
			g_stop_entropy_counters = &g_stop_entropy_counters;
			break;
		case 'r':
			g_reset_entropy_counters = &g_reset_entropy_counters;
			break;
		case 'h':
			g_help = &g_help;
			break;
		case 'o':
			g_output_file = optarg;
			break;
		case '?':
			error(0, 0, "%s:%d getopt_long returned value \"?\"", __FILE__, __LINE__);
			bad_input_parsed = 1;
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
	if(g_sign_message		!= NULL) action_count++;
	if(g_auth_message		!= NULL) action_count++;
	if(g_dump_random_numbers	!= NULL) action_count++;
	if(g_dump_entropy_counters_lo	!= NULL) action_count++;
	if(g_dump_entropy_counters_hi	!= NULL) action_count++;
	if(g_get_entropy_counters_state	!= NULL) action_count++;
	if(g_start_entropy_counters	!= NULL) action_count++;
	if(g_stop_entropy_counters	!= NULL) action_count++;
	if(g_reset_entropy_counters	!= NULL) action_count++;
	if(g_help			!= NULL) action_count++;
	
	if(action_count == 0) {
		puts(USAGE_STR);
		error(1, 0, "%s:%d no options parsed", __FILE__, __LINE__);
	}

	if(action_count > 1) {
		puts(USAGE_STR);
		error(1, 0, "%s:%d too many options parsed", __FILE__, __LINE__);
	}
}

void do_sign_message(void *validator_map) {
	
	int i;
	int infile_fd;
	int outfile_fd;
	int result;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// open the input file and attempt to read MESSAGE_IN_SIZE byte into
	// the MESSAGE_IN_OFFSET buffer in the validator
	//
	infile_fd = open(g_sign_message, O_RDONLY);
	if(infile_fd < 0) {
		perror("input file open");
		exit(EXIT_FAILURE);
	}
	
	result = read(infile_fd, &((unsigned char*)(validator_map))[MESSAGE_IN_OFFSET], MESSAGE_IN_SIZE);
	if(result < 0) {
		perror("input file read");
		exit(EXIT_FAILURE);
	}
	
	if(result != MESSAGE_IN_SIZE) {
		char *the_file = __FILE__;
		int the_line = __LINE__;
		error(0, 0, "%s:%d input file %s", the_file, the_line, g_sign_message);
		error(0, 0, "%s:%d did not provide %d bytes", the_file, the_line, MESSAGE_IN_SIZE);
		error(1, 0, "%s:%d only provided %d bytes", the_file, the_line, result);
	}
	
	result = close(infile_fd);
	if(result < 0) {
		perror("input file close");
		exit(EXIT_FAILURE);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = SIGN_MESSAGE_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed signature in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	printf("    version = %02X%02X\n", 
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  0],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  1]
	);
	
	printf("design hash = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  2],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  3],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  4],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  5],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  6],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  7],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  8],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET +  9],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 10],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 11]
	);
	
	printf("  unique id = %02X%02X%02X%02X%02X%02X%02X%02X\n", 
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 12],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 13],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 14],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 15],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 16],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 17],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 18],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 19]
	);
	
	printf("random salt = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 20],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 21],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 22],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 23],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 24],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 25],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 26],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 27],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 28],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 29],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 30],
		((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + 31]
	);
	
	printf("\ncomplete signed message:\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + i]);
	}
	printf("\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[MESSAGE_IN_OFFSET + i]);
	}
	printf("\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[HMAC_OUT_OFFSET + i]);
	}
	printf("\n\n");

	//
	// if an output file was passed in, write the results out to it
	//
	if(g_output_file != NULL) {
		outfile_fd = open(g_output_file, O_WRONLY | O_CREAT | O_SYNC);
		if(outfile_fd < 0) {
			perror("output file open");
			exit(EXIT_FAILURE);
		}

		result = write(outfile_fd, &((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET], MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE);
		if(result < 0) {
			perror("output file write");
			exit(EXIT_FAILURE);
		}
	
		if(result != MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE) {
			char *the_file = __FILE__;
			int the_line = __LINE__;
			error(0, 0, "%s:%d output file %s", the_file, the_line, g_output_file);
			error(0, 0, "%s:%d did not accept %d bytes", the_file, the_line, MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE);
			error(1, 0, "%s:%d only accepted %d bytes", the_file, the_line, result);
		}
	
		result = close(outfile_fd);
		if(result < 0) {
			perror("output file close");
			exit(EXIT_FAILURE);
		}
		
		printf("binary signed message written to output file:\n");
		printf("\'%s\"\n", g_output_file);
	} else {
		printf("no output file specified.\n");
	}
}

void do_auth_message(void *validator_map) {

	int i;
	int infile_fd;
	int result;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// open the input file and attempt to read MESSAGE_OUT_SIZE +
	// MESSAGE_IN_SIZE + HMAC_OUT_SIZE byte into the MESSAGE_OUT_OFFSET
	// buffer in the validator
	//
	infile_fd = open(g_auth_message, O_RDONLY);
	if(infile_fd < 0) {
		perror("input file open");
		exit(EXIT_FAILURE);
	}
	
	result = read(infile_fd, &((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET], MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE);
	if(result < 0) {
		perror("input file read");
		exit(EXIT_FAILURE);
	}
	
	if(result != MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE) {
		char *the_file = __FILE__;
		int the_line = __LINE__;
		error(0, 0, "%s:%d input file %s", the_file, the_line, g_auth_message);
		error(0, 0, "%s:%d did not provide %d bytes", the_file, the_line, MESSAGE_OUT_SIZE + MESSAGE_IN_SIZE + HMAC_OUT_SIZE);
		error(1, 0, "%s:%d only provided %d bytes", the_file, the_line, result);
	}
	
	result = close(infile_fd);
	if(result < 0) {
		perror("input file close");
		exit(EXIT_FAILURE);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = AUTHENTICATE_MESSAGE_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed authentication in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	if(((unsigned long*)(validator_map))[RESULT_OFFSET / 4] != 0x00) {
		printf("\nauthentication FAILED, the message is NOT authentic\n");
	} else {
		printf("\nauthentication SUCCEEDED, the message IS authentic\n");
	}

	printf("\ncomplete tested message:\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[MESSAGE_OUT_OFFSET + i]);
	}
	printf("\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[MESSAGE_IN_OFFSET + i]);
	}
	printf("\n");
	for(i = 0 ; i < 32 ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[HMAC_OUT_OFFSET + i]);
	}
	printf("\n\n");
}

void do_dump_random_numbers(void *validator_map) {
	
	int i;
	int outfile_fd;
	int result;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = DUMP_RANDOM_NUMBERS_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed random dump in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	printf("\nrandom data:\n");
	for(i = 0 ; i < UPPER_HALF_SIZE ; i++) {
		printf("%02X", ((unsigned char*)(validator_map))[UPPER_HALF_OFFSET + i]);
		if(((i + 1) % 32) == 0)
			printf("\n");
	}
	printf("\n");

	//
	// if an output file was passed in, write the results out to it
	//
	if(g_output_file != NULL) {
		outfile_fd = open(g_output_file, O_WRONLY | O_CREAT | O_SYNC);
		if(outfile_fd < 0) {
			perror("output file open");
			exit(EXIT_FAILURE);
		}

		result = write(outfile_fd, &((unsigned char*)(validator_map))[UPPER_HALF_OFFSET], UPPER_HALF_SIZE);
		if(result < 0) {
			perror("output file write");
			exit(EXIT_FAILURE);
		}
	
		if(result != UPPER_HALF_SIZE) {
			char *the_file = __FILE__;
			int the_line = __LINE__;
			error(0, 0, "%s:%d output file %s", the_file, the_line, g_output_file);
			error(0, 0, "%s:%d did not accept %d bytes", the_file, the_line, UPPER_HALF_SIZE);
			error(1, 0, "%s:%d only accepted %d bytes", the_file, the_line, result);
		}
	
		result = close(outfile_fd);
		if(result < 0) {
			perror("output file close");
			exit(EXIT_FAILURE);
		}
		
		printf("binary random data written to output file:\n");
		printf("\'%s\"\n", g_output_file);
	} else {
		printf("no output file specified.\n");
	}
}

void do_dump_entropy_counters_lo(void *validator_map) {
	
	int i;
	int outfile_fd;
	int result;
	char *out_str_ptr;
	char out_str[4096];
	int out_size;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = DUMP_ENTROPY_COUNTERS_LO_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed entropy counters lo dump in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	out_str_ptr = out_str;
	out_size = sizeof(out_str);
	result = snprintf(out_str_ptr, out_size, "\nlo entropy counters:\n");
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	result = snprintf(out_str_ptr, out_size, "value\tcount\n");
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	for(i = 0 ; i < UPPER_HALF_SIZE / 4 ; i++) {
		result = snprintf(out_str_ptr, out_size, "%d\t%lu\n", i, ((unsigned long*)(validator_map))[(UPPER_HALF_OFFSET / 4) + i]);
		if(result < 0) {
			perror("snprintf");
			exit(EXIT_FAILURE);
		}
		if(result > out_size) {
			out_str_ptr += out_size;
			out_size -= out_size;
		} else {
			out_str_ptr += result;
			out_size -= result;
		}
	}
	result = snprintf(out_str_ptr, out_size, "\ntransaction count %lu\n\n", ((unsigned long*)(validator_map))[(RESULT_OFFSET / 4)]);
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	printf("%s", out_str);
	
	//
	// if an output file was passed in, write the results out to it
	//
	if(g_output_file != NULL) {
		outfile_fd = open(g_output_file, O_WRONLY | O_CREAT | O_SYNC);
		if(outfile_fd < 0) {
			perror("output file open");
			exit(EXIT_FAILURE);
		}

		result = write(outfile_fd, out_str, out_str_ptr - out_str);
		if(result < 0) {
			perror("output file write");
			exit(EXIT_FAILURE);
		}
	
		if(result != (out_str_ptr - out_str)) {
			char *the_file = __FILE__;
			int the_line = __LINE__;
			error(0, 0, "%s:%d output file %s", the_file, the_line, g_output_file);
			error(0, 0, "%s:%d did not accept %d bytes", the_file, the_line, out_str_ptr - out_str);
			error(1, 0, "%s:%d only accepted %d bytes", the_file, the_line, result);
		}
	
		result = close(outfile_fd);
		if(result < 0) {
			perror("output file close");
			exit(EXIT_FAILURE);
		}
		
		printf("entropy counters lo written to output file:\n");
		printf("\'%s\"\n", g_output_file);
	} else {
		printf("no output file specified.\n");
	}
}

void do_dump_entropy_counters_hi(void *validator_map) {
	
	int i;
	int outfile_fd;
	int result;
	char *out_str_ptr;
	char out_str[4096];
	int out_size;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = DUMP_ENTROPY_COUNTERS_HI_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed entropy counters hi dump in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	out_str_ptr = out_str;
	out_size = sizeof(out_str);
	result = snprintf(out_str_ptr, out_size, "\nhi entropy counters:\n");
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	result = snprintf(out_str_ptr, out_size, "value\tcount\n");
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	for(i = 0 ; i < UPPER_HALF_SIZE / 4 ; i++) {
		result = snprintf(out_str_ptr, out_size, "%d\t%lu\n", i + 128, ((unsigned long*)(validator_map))[(UPPER_HALF_OFFSET / 4) + i]);
		if(result < 0) {
			perror("snprintf");
			exit(EXIT_FAILURE);
		}
		if(result > out_size) {
			out_str_ptr += out_size;
			out_size -= out_size;
		} else {
			out_str_ptr += result;
			out_size -= result;
		}
	}
	result = snprintf(out_str_ptr, out_size, "\ntransaction count %lu\n\n", ((unsigned long*)(validator_map))[(RESULT_OFFSET / 4)]);
	if(result < 0) {
		perror("snprintf");
		exit(EXIT_FAILURE);
	}
	if(result > out_size) {
		out_str_ptr += out_size;
		out_size -= out_size;
	} else {
		out_str_ptr += result;
		out_size -= result;
	}

	printf("%s", out_str);
	
	//
	// if an output file was passed in, write the results out to it
	//
	if(g_output_file != NULL) {
		outfile_fd = open(g_output_file, O_WRONLY | O_CREAT | O_SYNC);
		if(outfile_fd < 0) {
			perror("output file open");
			exit(EXIT_FAILURE);
		}

		result = write(outfile_fd, out_str, out_str_ptr - out_str);
		if(result < 0) {
			perror("output file write");
			exit(EXIT_FAILURE);
		}
	
		if(result != (out_str_ptr - out_str)) {
			char *the_file = __FILE__;
			int the_line = __LINE__;
			error(0, 0, "%s:%d output file %s", the_file, the_line, g_output_file);
			error(0, 0, "%s:%d did not accept %d bytes", the_file, the_line, out_str_ptr - out_str);
			error(1, 0, "%s:%d only accepted %d bytes", the_file, the_line, result);
		}
	
		result = close(outfile_fd);
		if(result < 0) {
			perror("output file close");
			exit(EXIT_FAILURE);
		}
		
		printf("entropy counters hi written to output file:\n");
		printf("\'%s\"\n", g_output_file);
	} else {
		printf("no output file specified.\n");
	}
}

void do_get_entropy_counters_state(void *validator_map) {
	
	int result;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = GET_ENTROPY_COUNTERS_STATE_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed get entropy counter state in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	if((((unsigned long*)(validator_map))[(RESULT_OFFSET / 4)]) != 0x00) {
		printf("\nEntropy counter is ENABLED.\n\n");
	} else {
		printf("\nEntropy counter is DISABLED.\n\n");
	}
}

void do_start_entropy_counters(void *validator_map) {
	
	int result;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = START_ENTROPY_COUNTERS_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed start entropy counter in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	printf("\nEntropy counter is STARTED.\n\n");
}

void do_stop_entropy_counters(void *validator_map) {
	
	int result;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = STOP_ENTROPY_COUNTERS_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed stop entropy counter in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	printf("\nEntropy counter is STOPPED.\n\n");
}

void do_reset_entropy_counters(void *validator_map) {
	
	int result;

	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	//
	// make sure the validator is ready for us to use
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(1, 0, "%s:%d validator hardware not ready", __FILE__, __LINE__);
	}
	
	//
	// set the ARM request field and flip the flag to Nios ownership
	//
	((unsigned long*)(validator_map))[ARM_REQ_OFFSET / 4] = RESET_ENTROPY_COUNTERS_REQ;
	((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] = NIOS_OWNS_FLAG;
	
	//
	// get the starting time and calculate a 500us timeout period
	//
	result = gettimeofday(&start_time, NULL);
	if(result < 0) {
		perror("gettimofday");
		exit(EXIT_FAILURE);
	}
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}
	
	//
	// wait for the validator to complete, flag flips to ARM ownership
	//
	do {
		result = gettimeofday(&end_time, NULL);
		if(result < 0) {
			perror("gettimofday");
			exit(EXIT_FAILURE);
		}
		if((end_time.tv_sec >= timeout_time.tv_sec) && (end_time.tv_usec >= timeout_time.tv_usec))
			break;
	} while(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG);
	
	//
	// calculate the time spent waiting for request
	//
	timeout_time.tv_sec = end_time.tv_sec - start_time.tv_sec;
	if(timeout_time.tv_sec == 0) {
		if(start_time.tv_sec > end_time.tv_sec)
			error(1, 0, "%s:%d start_time.tv_sec greater than end_time.tv_sec", __FILE__, __LINE__);
		else
			timeout_time.tv_usec = end_time.tv_usec - start_time.tv_usec;
	} else {
		timeout_time.tv_usec = end_time.tv_usec + 1000000 - start_time.tv_usec;
		if(timeout_time.tv_usec >= 1000000) {
			timeout_time.tv_usec -= 1000000;
		} else {
			timeout_time.tv_sec--;
		}
	}
	
	//
	// verify that we did not fall out of while loop due to timeout
	//
	if(((unsigned char*)(validator_map))[HS_FLAGS_OFFSET] != ARM_OWNS_FLAG) {
		error(0, 0, "%s:%d validator hardware timed out", __FILE__, __LINE__);
		error(1, 0, "%s:%d %u seconds, %u microseconds", __FILE__, __LINE__, (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	}
	
	//
	// print the statistics from the request
	//
	printf("\nvalidator completed reset entropy counter in %u seconds, %u microseconds\n", (unsigned int)timeout_time.tv_sec, (unsigned int)timeout_time.tv_usec);
	
	printf("\nEntropy counter has been reset.\n\n");
}

void do_help(void) {
	puts(HELP_STR);
	puts(USAGE_STR);
}

