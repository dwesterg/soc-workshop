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
#include <sys/mount.h>

#define EXPECTED_KERNEL_VERSION "3.10.31-ltsi"
#define VALIDATOR_INFO_FILE "ws2_validation_info.txt"
#define FAT_TMP_MOUNT "/tmp/ws2_tmpdir"
#define FAT_DEV_NAME "/dev/mmcblk0p1"

#define MAX_COMMAND_LENGTH	(4096 * 3)
#define PIPE_READ_FD		(0)
#define PIPE_WRITE_FD		(1)

void const *g_preparser_strings[] = {
        "FILE=" __FILE__,
        "DATE=" __DATE__,
        "TIME=" __TIME__
};

int main(int argc, char * argv[])
{
  
  FILE *fout_ptr, *fproc_version_ptr;
  char *version_string;
  size_t len = 0;
  ssize_t bytes_read;
  char kernel_version_build [12] ;
  char * pstr_start;
  char * pstr_mid;
  char * pstr_end;
  size_t str_len;
  char * user_name_build;
  char * host_name_build;
  char * kernel_build_date_time;
  struct stat st = {0};
  
  	int i;
	int stdout_pipefd[2];
	int stdin_pipefd[2];
	int validator_rand_fd;
	int validator_sign_fd;
	int signature_fd;
	char command_string[MAX_COMMAND_LENGTH];
	char command_output[MAX_COMMAND_LENGTH];
	ssize_t ssize_t_result;
	size_t size_t_result;
	int int_result;
	unsigned char hash_array[32];
	unsigned char salted_hash_array[32];
	char strtol_convert_array[2];
	unsigned char hi_nibble;
	unsigned char lo_nibble;
	unsigned char salt[32];
	unsigned char signature[96];


  
  if (argc != 1)
  {
    error(1, 0, "Validator application does not expect arguments.");
  }
  
  /* Dump argv to make gcc happy*/
  argv = argv;
  
  
    fout_ptr = fopen(VALIDATOR_INFO_FILE, "w");
      if (fout_ptr == NULL)
      { 
	error(1, 0, "Error opening temp file");
      }
    
    /* Open the kernel version information file. 
     * We use /proc/version in place of system calls to be
     * able to extract the username of the person who compiled
     * the kernel. */
    fproc_version_ptr = fopen("/proc/version", "r");
      if (fproc_version_ptr == NULL)
      { 
	fclose(fout_ptr);
	error(1, 0, "Error opening output file.");
      }

    /* Read the kernel version information. */
    bytes_read = getline(&version_string, &len, fproc_version_ptr);
      if (bytes_read == -1)
      {
	fclose(fproc_version_ptr);
	fclose(fout_ptr);
	error(1, 0, "Error reading kernel version");
      }
      
    fclose(fproc_version_ptr);
    
    /* Extract Kernel version & username/hostname */
    sscanf(version_string, "%*s %*s %s", kernel_version_build);
      
    if (strncmp(kernel_version_build, EXPECTED_KERNEL_VERSION, 12) !=0)
    {
      printf("Your kernel version did not match the expected kernel version.\n");
      printf("Found kernel version: %s\n", kernel_version_build);
      printf("Expected kernel version: %s\n", EXPECTED_KERNEL_VERSION);
      fclose(fout_ptr);
      error(1, 0, "Incorrect kernel version\n");
    }


    /* Extract the username of the user who compiled the kernel and the 
     * hostname on which they compiled the kernel. */
    pstr_start = strstr(version_string, "(") + 1;
    pstr_mid = strstr(version_string, "@");
    pstr_end = strstr(version_string, ")");
    
    /* Oneline error collection */
    if (pstr_start == NULL || pstr_mid == NULL || pstr_end == NULL)
    {
	fclose(fout_ptr);
      	error(1, 0, "Error reading kernel complier user or host name");
    }

    
    str_len = pstr_mid - pstr_start;
    
    user_name_build = (char *) malloc(str_len + 1);
    strncpy(user_name_build, pstr_start, str_len);
    user_name_build[str_len] = '\0';
    
    
    pstr_mid++;
    str_len = pstr_end - pstr_mid;
    
    host_name_build = (char *) malloc(str_len + 1);
    strncpy(host_name_build, pstr_mid, str_len);
    host_name_build[str_len] = '\0';
    
    
    
    /* Extract the kernel compilation date & time. */
    pstr_start = strstr(version_string, "SMP") + 4;
    
    /* version_string includes trailing newline */
    str_len = strlen(version_string) - *pstr_start - 1;
    
    kernel_build_date_time = (char *) malloc(str_len + 1);
    strncpy(kernel_build_date_time, pstr_start, str_len);
    kernel_build_date_time[str_len] = '\0';
    
    /* Print the kernel build information and output it to the temp file */
    printf("User Name: %s\n", user_name_build);
    printf("Host Name: %s\n", host_name_build);
    printf("Kernel build date: %s", kernel_build_date_time);
    
    fprintf(fout_ptr, "Kernel Build Information:\n");
    fprintf(fout_ptr, "  Built by: %s\n", user_name_build);
    fprintf(fout_ptr, "  Built on host: %s\n", host_name_build);
    fprintf(fout_ptr, "  Built on date: %s\n", kernel_build_date_time);

    
    /* Mount FAT file system.  Mount it independent of any user mounts. */
    if (stat(FAT_TMP_MOUNT, &st) == -1) 
    {
      if (mkdir(FAT_TMP_MOUNT, 0700) == -1)
      {
	fclose(fout_ptr);
	error(1, 0, "Failed to create temporary mount directory");
	}
    }
    
    if (mount(FAT_DEV_NAME, FAT_TMP_MOUNT, "vfat", 0, "") == -1)
    {
      fclose(fout_ptr);
      error(1, 0, "Failed to mount FAT file system");
    }
    
    
    /* Verify the generated files exist where expected on the FAT partition */      
    if (stat(FAT_TMP_MOUNT "/u-boot.img", &st) == -1)
    {
      printf("Did not find expected u-boot image on FAT file system.\n");
      if (umount(FAT_TMP_MOUNT) == -1)
      {
	fclose(fout_ptr);
	error(1, 0, "Failed to unmount FAT file system");
      }
      fclose(fout_ptr);
      error(1, 0, "Failed u-boot image verify");
    }

    if (stat(FAT_TMP_MOUNT "/u-boot.scr", &st) == -1)
    {
      printf("Did not find expected u-boot script on FAT file system.\n");
      if (umount(FAT_TMP_MOUNT) == -1)
      {
	fclose(fout_ptr);
	error(1, 0, "Failed to unmount FAT file system");
      }
      fclose(fout_ptr);
      error(1, 0, "Failed u-boot script verify");
    }

    if (stat(FAT_TMP_MOUNT "/soc_system.dtb", &st) == -1)
    {
      printf("Did not find expected device tree image on FAT file system.\n");
      if (umount(FAT_TMP_MOUNT) == -1)
      {
	fclose(fout_ptr);
	error(1, 0, "Failed to unmount FAT file system");
      }
      fclose(fout_ptr);
      error(1, 0, "Failed device image verify");
    }
    
    fprintf(fout_ptr, "All FAT images verified");
    
    fclose(fout_ptr);
    
    
    /* Unmount FAT file system before hitting Rod's stuff, in case of error. */
    
      if (umount(FAT_TMP_MOUNT) == -1)
      {
	fclose(fout_ptr);
	error(1, 0, "Failed to unmount FAT file system");
      }
    

     if (rmdir(FAT_TMP_MOUNT) == -1)
    {
      error(1, 0, "Failed to remove temporary mount directory");
    }


    
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
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s > /dev/null 2>&1", "mkdir", "ws2_validation_archive");
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
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s %s %s > /dev/null 2>&1", "cp", argv[0], VALIDATOR_INFO_FILE, "validation_signature.bin", "ws2_validation_archive");
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
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s %s > /dev/null 2>&1", "tar", "cf", "ws2_validation_archive.tar", "ws2_validation_archive");
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
		
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s > /dev/null 2>&1", "rm", "-rf", "ws2_validation_archive");
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
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s > /dev/null 2>&1", "gzip", "ws2_validation_archive.tar");
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
	int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s > /dev/null 2>&1", "/examples/validator/sign_file.sh", "/examples/validator/validator_sign.sh", "ws2_validation_archive.tar.gz");
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

	int_result = unlink(VALIDATOR_INFO_FILE);
	if (int_result == -1)
		error_at_line(1, errno, __FILE__, __LINE__, "failed to unlink file");

	
	/* Copy WS files to FAT file system. */ 
        /* Mount FAT file system.  Mount it independent of any user mounts. */
    if (stat(FAT_TMP_MOUNT, &st) == -1) 
    {
      if (mkdir(FAT_TMP_MOUNT, 0700) == -1)
      {
	error(1, 0, "Failed to create temporary mount directory");
	}
    }
    
    if (mount(FAT_DEV_NAME, FAT_TMP_MOUNT, "vfat", 0, "") == -1)
    {
      error(1, 0, "Failed to mount FAT file system");
    }

    int_result = snprintf(command_string, MAX_COMMAND_LENGTH, "%s %s %s > /dev/null 2>&1", "cp", "ws2_validation_archive.tar.gz*", FAT_TMP_MOUNT);
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
		


     
     if (umount(FAT_TMP_MOUNT) == -1)
     {
       error(1, 0, "Failed to unmount FAT file system");
     }

     
     if (rmdir(FAT_TMP_MOUNT) == -1)
     {
      error(1, 0, "Failed to remove temporary mount directory");
     }
    
	printf("Validation Complete\n");
	printf("Please forward these validation files to prove that you completed the lab work:\n%s\n%s\n", "validation_archive.tar.gz", "validation_archive.tar.gz.sign");   

}


