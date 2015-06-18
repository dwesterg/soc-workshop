/*
 * Copyright (c) 2013, Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Altera Corporation nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// bring in the view from the processor
#include "hps_0.h"
#include "socal/hps.h"
// bring in the view from the dmas
#include "soc_system.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "sgdma_dispatcher.h"
#include "descriptor_regs.h"
#include <sys/mman.h>
#include <math.h>
#include "string.h"

#define SGDMA_TO_FFT_CSR_BASE ((int)mappedBase + FFT_SUB_SGDMA_TO_FFT_CSR_BASE)
#define SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE ((int)mappedBase + FFT_SUB_SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE)
#define SGDMA_FROM_FFT_CSR_BASE ((int)mappedBase + FFT_SUB_SGDMA_FROM_FFT_CSR_BASE)
#define SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE ((int)mappedBase + FFT_SUB_SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE)
#define DATA_BASE (FFT_SUB_DATA_BASE + (int)mappedBase)
#define RESULT_BASE (FFT_SUB_DATA_BASE + (int)mappedBase +(FFT_SUB_DATA_SPAN/2))
// the FFT csr register from the point of view of the processor
#define FFT_CSR_BASE ((int)mappedBase + FFT_SUB_FFT_STADAPTER_0_BASE)

// this is the onchip ram base from the DMA's point of view
#define  DMA_DATA_BASE  FFT_SUB_SGDMA_FROM_FFT_FFT_SUB_DATA_BASE
#define  DMA_RESULT_BASE  (FFT_SUB_SGDMA_FROM_FFT_FFT_SUB_DATA_BASE + (FFT_SUB_SGDMA_FROM_FFT_FFT_SUB_DATA_SPAN/2))

// this is the physical address of lw bridge.
//#define BASE_ADDRESS 0xff200000
//mappedBase is the linux view of the same.

// main creates 2 files. the input value and the real part of the fft.
// these are used in the web browser app.

int main(int argc, char **argv)
{
	int i;
	int mem;
	int waveform = 0;
	volatile unsigned int *value, real, image;
	unsigned int temp;
	void *mappedBase;	// where linux sees the lw bridge.
	FILE *pFile;
	FILE *pFilei;
	FILE *pFile_in;

	//can be 32 - 4K but must be a power of 2
	// *****************        feel free to change this value  ****************
	// 128 256 512 1024 2048 4096
	int data_length = 128;

	sgdma_standard_descriptor descriptor0, descriptor1;

	printf("\n\nHello from SoC FPGA to everyone!\n");
	printf("This program was called with \"%s\".\n", argv[0]);
	pFile = fopen("/www/pages/samples.js", "w");
	pFilei = fopen("/www/pages/samplesi.js", "w");
	pFile_in = fopen("/www/pages/source.js", "w");

	// just parsing the input parameters
	if (argc > 1) {
		if (strcmp(argv[1], "0") == 0) {
			printf(" **** square wave **** \n");
			waveform = 0;
		}
		if (strcmp(argv[1], "1") == 0) {
			printf(" **** sine wave ****\n");
			waveform = 1;
		}
	} else {
		printf("The command had no other arguments.\n");
		printf(" **** square wave **** \n");
	}

	// look for the length
	if (argc > 2) {
		data_length = atoi(argv[2]);
		switch (data_length) {
		case 128:
		case 256:
		case 512:
		case 1024:
		case 2048:
		case 4096:
			break;
		default:
			printf("%s %d must be a power of 2 defaulting to 128\n", argv[2], data_length);
			data_length = 128;
			break;

		}
	}
	// we need to get a pointer to the LW_BRIDGE from the softwares point of view.  

	// need to open a file.
	/* Open /dev/mem */
	if ((mem = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
		fprintf(stderr, "Cannot open /dev/mem\n"), exit(1);
	// now map it into lw bridge space
	mappedBase = mmap(0, 0x1f0000, PROT_READ | PROT_WRITE, MAP_SHARED, mem, ALT_LWFPGASLVS_OFST);

	if (mappedBase == (void *)-1) {
		printf("Memory map failed. error %i\n", (int)mappedBase);
		perror("mmap");
	}
	// now that the fpga space is mapped we need to clear out the onchip ram so it is ready for data

	// load the right wave form into memory and make a copy in a file for display purposes.
	printf("samples_size=%d\r\n", data_length);
	fprintf(pFile_in, "source_size=%d\r\n", data_length);
	fprintf(pFile_in, "source=[");
	value = (unsigned int *)(DATA_BASE);
	for (i = 0; i < data_length; i++) {
		if (waveform == 0)
			temp = ((i % 32) < 16) ? 0x7fff : 0x8000;	// make small spikes  should give a sinx/x wave form. if I remember my DSP well

		if (waveform == 1)
			temp = 0xffff & (int)(((sin(2.0 * 3.1415 * (float)i / 16.0) * 0x7fff)));	//need to store in bigendian format

		value[i] = temp;	//<<16;// need to be in the lower 16 bits so it is the real part
		printf("signal 0x%x %i\n", i, (int)(short)temp);
		fprintf(pFile_in, "%d, ", (int)(short)temp);	// stick in a file to be displayed.
	}
	fprintf(pFile_in, "]\n\r");
	fclose(pFile_in);

	// zero out the results area              
	value = (unsigned int *)((int)RESULT_BASE);
	for (i = 0; i < data_length * 2; i++) {
		value[i] = 0;	// make small spikes  should give a sinx/x wave form. if I remember my DSP well
	}

	// need to tell the FFT how long the fft is
	*(unsigned long *)FFT_CSR_BASE = data_length;
	// now do the real work
	construct_standard_mm_to_st_descriptor(&descriptor0, (alt_u32 *) DMA_DATA_BASE, data_length * 4, DESCRIPTOR_CONTROL_GENERATE_SOP_MASK | DESCRIPTOR_CONTROL_GENERATE_EOP_MASK);
	construct_standard_st_to_mm_descriptor(&descriptor1, (alt_u32 *) DMA_RESULT_BASE, data_length * 8, DESCRIPTOR_CONTROL_END_ON_EOP_MASK);
	// now write the constructors to memory
	write_standard_descriptor(SGDMA_TO_FFT_CSR_BASE, SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE, &descriptor0);
	write_standard_descriptor(SGDMA_FROM_FFT_CSR_BASE, SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE, &descriptor1);
	// could just poll here
	usleep(500);
	if ((unsigned int)read_csr_status(SGDMA_TO_FFT_CSR_BASE) != 2)
		printf("ERROR sgdma to fft sgdma status 0x%x ( should be 2)\n", (unsigned int)read_csr_status(SGDMA_TO_FFT_CSR_BASE));
	if ((unsigned int)read_csr_status(SGDMA_FROM_FFT_CSR_BASE) != 2)
		printf("ERROR sgdma from fft sgdma status 0x%x ( should be 2)\n", (unsigned int)read_csr_status(SGDMA_FROM_FFT_CSR_BASE));

	// now read the results from memory
	value = (unsigned int *)((int)RESULT_BASE);
	fprintf(pFile, "samples_size=%d\r\n", data_length);
	fprintf(pFile, "samples=[");
	fprintf(pFilei, "samplesi_size=%d\r\n", data_length);
	fprintf(pFilei, "samplesi=[");

	printf("\r\n\r\nfft =  real,  image\r\n");
	for (i = 0; i < data_length; i++) {
		real = value[2 * i];
		image = value[2 * i + 1];
		printf("fft = %x %i %i\r\n", i, real, image);
		fprintf(pFile, "%d,", real);
		fprintf(pFilei, "%d,", image);
	}

	value = (unsigned int *)((int)mappedBase + SYSID_QSYS_BASE);
	fprintf(pFile, "]\n\r");
	fprintf(pFilei, "]\n\r");

	// close everything up
	fclose(pFile);
	fclose(pFilei);

	munmap(mappedBase, 0x1f0000);
	close(mem);

	return 0;
}

