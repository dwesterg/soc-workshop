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

/*
 * The diagnostic program goes through a few steps to test if the board is 
 * working properly
 *
 * 1. Driver Installation Check
 *
 * 2. Board Installation Check
 *
 * 3. Basic Functionality Check
 *
 * 4. Buffer read/write test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "CL/cl.h"

#define STRING_RETURN_SIZE 1024
#define SHARED_BUFFER_SIZE 16*1024*0124


// ACL runtime configuration
static cl_platform_id platform;
static cl_device_id device;
static cl_context context;
static cl_command_queue queue;
static cl_kernel kernel;
static cl_program program;
static cl_int status;

// free the resources allocated during initialization
static void freeResources() {
  if(kernel) 
    clReleaseKernel(kernel);  
  if(program) 
    clReleaseProgram(program);
  if(queue) 
    clReleaseCommandQueue(queue);
  if(context) 
    clReleaseContext(context);
}


static void dump_error(const char *str, cl_int status) {
  printf("%s\n", str);
  printf("Error code: %d\n", status);
  freeResources();
  exit(-1);
}

void ocl_device_init()
{
  char buf[1000];

  cl_uint num_platforms=0;
  cl_uint num_devices;
  
  // get the platform ID
  status = clGetPlatformIDs(0, NULL, &num_platforms);
  if(status != CL_SUCCESS) dump_error("Failed clGetPlatformIDs.", status);
  
  status = clGetPlatformIDs(1, &platform, NULL);
  if(status != CL_SUCCESS) dump_error("Failed clGetPlatformIDs.", status);

  char platform_name[256];
  clGetPlatformInfo (platform,CL_PLATFORM_NAME, 256, platform_name, NULL);
  printf("Using platform: %s\n", platform_name);

  // get the device ID
  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &num_devices);
  if(status != CL_SUCCESS) dump_error("Failed clGetDeviceIDs.", status);

  status = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buf), (void*)&buf, NULL);
  status = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buf), (void*)&buf, NULL);

  // create a context
  context = clCreateContext(0, 1, &device, NULL, NULL, &status);
  if(status != CL_SUCCESS) dump_error("Failed clCreateContext.", status);

  // create a command queue
  queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
  if(status != CL_SUCCESS) dump_error("Failed clCreateCommandQueue.", status);
}


int general_basic_tests()
{
  if ( system("cat /proc/modules | grep \"aclsoc_drv\" > /dev/null") ) {
    printf("\nUnable to find the kernel mode driver.\n");
    printf("\nPlease make sure you have properly installed arm32 RTE and loaded the driver:\n");
    printf("\n   insmod aclsoc_drv.ko\n");
    return -1;
  }
  
  // Try to open manually, to see if the char device is even there.
  static char dev_name[16] = "/dev/acl0";
  ssize_t f = open (dev_name, O_RDWR);
  if( f == -1 ) {
    printf ("Board name %s is not available\n", dev_name);
    return -1;
  }
  close (f);

  printf("\nVerified that the kernel mode driver is installed on the host machine.\n\n");
  return 0;
}

int scan_device()
{
  ocl_device_init();

  static char vendor_name[STRING_RETURN_SIZE];
  status = clGetDeviceInfo (device, CL_DEVICE_VENDOR, STRING_RETURN_SIZE, vendor_name, NULL);
  if(status != CL_SUCCESS) dump_error("Failed clGetDeviceInfo(CL_DEVICE_VENDOR)", status);
  printf ("Board vendor name: %s\n", vendor_name);

  // get all supported board names from MMD
  static char board_name[STRING_RETURN_SIZE];
  status = clGetDeviceInfo (device, CL_DEVICE_NAME, STRING_RETURN_SIZE, board_name, NULL);
  if(status != CL_SUCCESS) dump_error("Failed clGetDeviceInfo(CL_DEVICE_NAME)", status);
  printf ("Board name: %s\n\n", board_name);
  

  char *shared_buf1 = (char*)malloc(SHARED_BUFFER_SIZE * sizeof(char));
  char *shared_buf2 = (char*)malloc(SHARED_BUFFER_SIZE * sizeof(char));
  if (shared_buf1 == NULL || shared_buf2 == NULL) {
    dump_error("Failed to allocate two buffers with malloc", 0);
  }
  
  int i;
  for (i = 0; i < SHARED_BUFFER_SIZE; i++) {
    shared_buf1[i] = i;
    shared_buf2[i] = -1;
  }
  
  cl_mem in1 = clCreateBuffer (context, CL_MEM_READ_WRITE, SHARED_BUFFER_SIZE, NULL, &status);
  if(status != CL_SUCCESS) dump_error("Failed clCreateBuffer for in1", status);
  
  status = clEnqueueWriteBuffer (queue, in1, CL_TRUE, 0, SHARED_BUFFER_SIZE, shared_buf1, 0, NULL, NULL);
  if (status != CL_SUCCESS) dump_error("Could not launch clEnqueueWriteBuffer!", status);
  
  status = clEnqueueReadBuffer (queue, in1, CL_TRUE, 0, SHARED_BUFFER_SIZE, shared_buf2, 0, NULL, NULL);
  if (status != CL_SUCCESS) dump_error("Could not launch clEnqueueWriteBuffer!", status);
  
  for (i = 0; i < SHARED_BUFFER_SIZE; i++) {
    if (shared_buf1[i] != shared_buf2[i] ) {
      printf("\nBuffer comparison failed!\n");
      printf ("#%d: %d vs %d\n", i, shared_buf1[i], shared_buf2[i]);
      freeResources();
      return -1;
    }
  }
  printf ("Buffer read/write test passed.\n");

  free (shared_buf1);
  free (shared_buf2);
  clReleaseMemObject (in1);
  freeResources();
  return 0;
}

int main()
{
  if( general_basic_tests() != 0 ) {
    printf("\nDIAGNOSTIC_FAILED\n");
    return 1;
  }
  if( scan_device() != 0 ){
    printf("\nDIAGNOSTIC_FAILED\n");
    return 1;
  }

  printf("\nDIAGNOSTIC_PASSED\n");
  return 0;
}

