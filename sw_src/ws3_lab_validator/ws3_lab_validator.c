#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_COMMAND_LENGTH	(4096)
#define PIPE_READ_FD		(0)
#define PIPE_WRITE_FD		(1)

void const *g_preparser_strings[] = {
        "FILE=" __FILE__,
        "DATE=" __DATE__,
        "TIME=" __TIME__
};

int main(int argc, char *argv[])
{
	int i;
	int stdout_pipefd[2];
	int stdin_pipefd[2];
	int validator_rand_fd;
	int validator_sign_fd;
	int signature_fd;
	int module_fd;
	char command_string[MAX_COMMAND_LENGTH];
	char command_output[MAX_COMMAND_LENGTH];
	long syscall_result;
	off_t off_t_result;
	ssize_t ssize_t_result;
	size_t size_t_result;
	int int_result;
	void *module_load_buffer;
	off_t module_size;
	const char *str_ptr;
	unsigned char hash_array[32];
	unsigned char salted_hash_array[32];
	char strtol_convert_array[2];
	unsigned char hi_nibble;
	unsigned char lo_nibble;
	unsigned char salt[32];
	unsigned char signature[96];

	/*
	 * validate the command line format
	 */
	if (argc != 1)
		error_at_line(1, 0, __FILE__, __LINE__, "program expects no arguments");

	/*
	 * remove any existing lab modules from the kernel
	 */
	syscall_result = syscall(SYS_delete_module, "lab_module", O_NONBLOCK);
	if (syscall_result != 0)
		if (errno != ENOENT)
			error_at_line(1, errno, __FILE__, __LINE__, "failed to delete module");

	syscall_result = syscall(SYS_delete_module, "my_uio_pdrv_genirq", O_NONBLOCK);
	if (syscall_result != 0)
		if (errno != ENOENT)
			error_at_line(1, errno, __FILE__, __LINE__, "failed to delete module");

	/*
	 * insert lab_module into the kernel
	 */
	module_fd = open("lab_module.ko", O_RDONLY | O_SYNC);
	if (module_fd < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to open module");
		
	module_size = lseek(module_fd, 0, SEEK_END);
	if (module_size == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to seek end module");
	
	module_load_buffer = malloc(module_size);
	if (module_load_buffer == NULL)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to malloc module storage");

	off_t_result = lseek(module_fd, 0, SEEK_SET);
	if (off_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to seek set module");

	ssize_t_result = read(module_fd, module_load_buffer, module_size);
	if (ssize_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to read module");
	
	int_result = close(module_fd);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close module");
	
	syscall_result = syscall(SYS_init_module, module_load_buffer, module_size, "");
	if (syscall_result != 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to init module");
		
	free(module_load_buffer);
	
	/*
	 * run lab_module_test
	 */
	int_result = pipe(stdout_pipefd);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to create pipe");
	
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s >&%d 2>&1", "./lab_module_test", stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
	
	ssize_t_result = read(stdout_pipefd[PIPE_READ_FD], command_output, MAX_COMMAND_LENGTH);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error reading command output from pipe");
	if (ssize_t_result == 0)
		error_at_line(1, 0, __FILE__, __LINE__, "no command output from pipe available");
	if (ssize_t_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command output overflow buffer");
	
	int_result = close(stdout_pipefd[PIPE_READ_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe read fd");
	
	int_result = close(stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe write fd");
	
	str_ptr = "Success\nEnd of test...\n";
	size_t_result = strlen(str_ptr);
	if (size_t_result != (unsigned int)ssize_t_result)
		error_at_line(1, 0, __FILE__, __LINE__, "unexpected command output length");

	command_output[ssize_t_result] = '\0';
	int_result = strncmp(str_ptr, command_output, size_t_result);
	if (int_result != 0)
		error_at_line(1, 0, __FILE__, __LINE__, "unexpected command output pattern");
	
	/*
	 * remove lab_module from the kernel
	 */
	syscall_result = syscall(SYS_delete_module, "lab_module", O_NONBLOCK);
	if (syscall_result != 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to delete module");
		
	/*
	 * insert my_uio_pdrv_genirq into the kernel
	 */
	module_fd = open("my_uio_pdrv_genirq.ko", O_RDONLY | O_SYNC);
	if (module_fd < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to open module");
		
	module_size = lseek(module_fd, 0, SEEK_END);
	if (module_size == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to seek end module");
	
	module_load_buffer = malloc(module_size);
	if (module_load_buffer == NULL)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to malloc module storage");

	off_t_result = lseek(module_fd, 0, SEEK_SET);
	if (off_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to seek set module");

	ssize_t_result = read(module_fd, module_load_buffer, module_size);
	if (ssize_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to read module");
	
	int_result = close(module_fd);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close module");
	
	syscall_result = syscall(SYS_init_module, module_load_buffer, module_size, "");
	if (syscall_result != 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to init module");
		
	free(module_load_buffer);
	
	/*
	 * run uio_module_test
	 */
	int_result = pipe(stdout_pipefd);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to create pipe");
	
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s >&%d 2>&1", "./uio_module_test", stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
	
	ssize_t_result = read(stdout_pipefd[PIPE_READ_FD], command_output, MAX_COMMAND_LENGTH);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error reading command output from pipe");
	if (ssize_t_result == 0)
		error_at_line(1, 0, __FILE__, __LINE__, "no command output from pipe available");
	if (ssize_t_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command output overflow buffer");
	
	int_result = close(stdout_pipefd[PIPE_READ_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe read fd");
	
	int_result = close(stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe write fd");
	
	str_ptr = "Success\nEnd of test...\n";
	size_t_result = strlen(str_ptr);
	if (size_t_result != (unsigned int)ssize_t_result)
		error_at_line(1, 0, __FILE__, __LINE__, "unexpected command output length");

	command_output[ssize_t_result] = '\0';
	int_result = strncmp(str_ptr, command_output, size_t_result);
	if (int_result != 0)
		error_at_line(1, 0, __FILE__, __LINE__, "unexpected command output pattern");
	
	/*
	 * remove my_uio_pdrv_genirq from the kernel
	 */
	syscall_result = syscall(SYS_delete_module, "my_uio_pdrv_genirq", O_NONBLOCK);
	if (syscall_result != 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to delete module");
		
	/*
	 * calculate argv[0] hash
	 */
	int_result = pipe(stdout_pipefd);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to create pipe");
	
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s >&%d 2>&1", "sha256sum", argv[0], stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
	
	ssize_t_result = read(stdout_pipefd[PIPE_READ_FD], command_output, MAX_COMMAND_LENGTH);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error reading command output from pipe");
	if (ssize_t_result == 0)
		error_at_line(1, 0, __FILE__, __LINE__, "no command output from pipe available");
	if (ssize_t_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command output overflow buffer");
	
	int_result = close(stdout_pipefd[PIPE_READ_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe read fd");
	
	int_result = close(stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe write fd");

	size_t_result = strspn(command_output, "0123456789abcdefABCDEF");
	if (size_t_result != 64)
		error_at_line(1, 0, __FILE__, __LINE__, "incorrect hash length output");
		
	for (i = 0 ; i < 32 ; i++) {
		strtol_convert_array[0] = command_output[i * 2];
		strtol_convert_array[1] = '\0';
		hi_nibble = (unsigned char)strtol(strtol_convert_array, NULL, 16);

		strtol_convert_array[0] = command_output[(i * 2) + 1];
		strtol_convert_array[1] = '\0';
		lo_nibble = (unsigned char)strtol(strtol_convert_array, NULL, 16);
		
		hash_array[i] = ((hi_nibble & 0x0F) << 4) | (lo_nibble & 0x0F);
	}

	/*
	 * modprobe validator_module
	 */
	int_result = system("modprobe validator_module > /dev/null 2>&1");
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
	
	
	/*
	 * acquire 256 bit salt value
	 */
	validator_rand_fd = open("/dev/validator_rand", O_RDONLY | O_SYNC);
	if (validator_rand_fd < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to open validator_rand");
		
	ssize_t_result = read(validator_rand_fd, salt, 32);
	if (ssize_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to read validator_rand");
	if (ssize_t_result != 32)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to read 32 from validator_rand");
	
	int_result = close(validator_rand_fd);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close validator_rand");

	/*
	 * create salted hash
	 */
	int_result = pipe(stdout_pipefd);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to create pipe");
	
	int_result = pipe(stdin_pipefd);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to create pipe");
	
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s <&%d >&%d 2>&1", "sha256sum", stdin_pipefd[PIPE_READ_FD], stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	ssize_t_result = write(stdin_pipefd[PIPE_WRITE_FD], hash_array, 32);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error writing command input to pipe");
	if (ssize_t_result != 32)
		error_at_line(1, 0, __FILE__, __LINE__, "error writing 32 byte command input to pipe");

	ssize_t_result = write(stdin_pipefd[PIPE_WRITE_FD], salt, 32);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error writing command input to pipe");
	if (ssize_t_result != 32)
		error_at_line(1, 0, __FILE__, __LINE__, "error writing 32 byte command input to pipe");

	int_result = close(stdin_pipefd[PIPE_WRITE_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe write fd");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
	
	int_result = close(stdin_pipefd[PIPE_READ_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe read fd");

	ssize_t_result = read(stdout_pipefd[PIPE_READ_FD], command_output, MAX_COMMAND_LENGTH);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error reading command output from pipe");
	if (ssize_t_result == 0)
		error_at_line(1, 0, __FILE__, __LINE__, "no command output from pipe available");
	if (ssize_t_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command output overflow buffer");
	
	int_result = close(stdout_pipefd[PIPE_READ_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe read fd");
	
	int_result = close(stdout_pipefd[PIPE_WRITE_FD]);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close pipe write fd");

	size_t_result = strspn(command_output, "0123456789abcdefABCDEF");
	if (size_t_result != 64)
		error_at_line(1, 0, __FILE__, __LINE__, "incorrect hash length output");
		
	for (i = 0 ; i < 32 ; i++) {
		strtol_convert_array[0] = command_output[i * 2];
		strtol_convert_array[1] = '\0';
		hi_nibble = (unsigned char)strtol(strtol_convert_array, NULL, 16);

		strtol_convert_array[0] = command_output[(i * 2) + 1];
		strtol_convert_array[1] = '\0';
		lo_nibble = (unsigned char)strtol(strtol_convert_array, NULL, 16);
		
		salted_hash_array[i] = ((hi_nibble & 0x0F) << 4) | (lo_nibble & 0x0F);
	}

	/*
	 * create signature
	 */
	validator_sign_fd = open("/dev/validator_sign", O_RDWR | O_SYNC);
	if (validator_sign_fd < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to open validator_sign");
		
	ssize_t_result = write(validator_sign_fd, salted_hash_array, 32);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error writing validator_sign");
	if (ssize_t_result != 32)
		error_at_line(1, 0, __FILE__, __LINE__, "error writing 32 byte to validator_sign");

	ssize_t_result = read(validator_sign_fd, signature, 96);
	if (ssize_t_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to read validator_sign");
	if (ssize_t_result != 96)
		error_at_line(1, 0, __FILE__, __LINE__, "failed to read 96 from validator_sign");
	
	int_result = close(validator_sign_fd);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close validator_sign");

	/*
	 * write signature file
	 */
	signature_fd = open("validation_signature.bin", O_WRONLY | O_CREAT | O_SYNC);
	if (signature_fd < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to open validation_signature");
		
	ssize_t_result = write(signature_fd, salt, 32);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error writing validation_signature");
	if (ssize_t_result != 32)
		error_at_line(1, 0, __FILE__, __LINE__, "error writing 32 byte to validation_signature");

	ssize_t_result = write(signature_fd, signature, 96);
	if (ssize_t_result < 0)
		error_at_line(1, errno, __FILE__, __LINE__, "error writing validation_signature");
	if (ssize_t_result != 96)
		error_at_line(1, 0, __FILE__, __LINE__, "error writing 96 byte to validation_signature");

	int_result = close(signature_fd);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to close validation_signature");

	/*
	 * archive relevant artifacts
	 */
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s > /dev/null 2>&1", "mkdir", "validation_archive");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s %s %s %s %s %s > /dev/null 2>&1", "cp", argv[0], "lab_module.ko", "lab_module_test", "my_uio_pdrv_genirq.ko", "uio_module_test", "validation_signature.bin", "validation_archive");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s %s > /dev/null 2>&1", "tar", "cf", "validation_archive.tar", "validation_archive");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s > /dev/null 2>&1", "rm", "-rf", "validation_archive");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");

	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
		
	/*
	 * compress archive
	 */
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s > /dev/null 2>&1", "gzip", "validation_archive.tar");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");
	
	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}
		
	/*
	 * sign archive
	 */
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s > /dev/null 2>&1", "/examples/validator/sign_file.sh", "/examples/validator/validator_sign.sh", "validation_archive.tar.gz");
	if (int_result >= MAX_COMMAND_LENGTH)
		error_at_line(1, 0, __FILE__, __LINE__, "command string truncated");
	
	int_result = system(command_string);
	if (int_result == -1)
		error_at_line(1, 0, __FILE__, __LINE__, "unable to run command");
		
	if(WIFEXITED(int_result)) {
		if(WEXITSTATUS(int_result) != 0)
			error_at_line(1, 0, __FILE__, __LINE__, "command returned nonzero exit status");
	} else {
		error_at_line(1, 0, __FILE__, __LINE__, "command exited abnormally");
	}

	/*
	 * delete intermediate debris
	 */
	int_result = unlink("validation_signature.bin");
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to unlink file");

	printf("Validation Complete\n");
	printf("Please forward these validation files to prove that you completed the lab work:\n%s\n%s\n", "validation_archive.tar.gz", "validation_archive.tar.gz.sign");
}

