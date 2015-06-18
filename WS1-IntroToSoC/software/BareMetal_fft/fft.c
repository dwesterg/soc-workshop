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
#include "hps_0.h"
// bring in the view from the processor
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
#include "csr_regs.h"
#include <math.h>
#include "string.h"
#include "socal/socal.h"
#include <stdint.h>

// defines for all the differing address spaces
// the porcessor sees the peripherals through the lw bridge which is located at address 0xff20_0000 == ALT_LWFPGASLVS_OFST
#define SGDMA_TO_FFT_CSR_BASE ((int)ALT_LWFPGASLVS_OFST + FFT_SUB_SGDMA_TO_FFT_CSR_BASE)
#define SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE ((int)ALT_LWFPGASLVS_OFST + FFT_SUB_SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE)
#define SGDMA_FROM_FFT_CSR_BASE ((int)ALT_LWFPGASLVS_OFST + FFT_SUB_SGDMA_FROM_FFT_CSR_BASE)
#define SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE ((int)ALT_LWFPGASLVS_OFST + FFT_SUB_SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE)
//this is the onchip ram base address from the processors point of view
#define DATA_BASE (FFT_SUB_DATA_BASE + ALT_LWFPGASLVS_OFST)
#define RESULT_BASE (FFT_SUB_DATA_BASE + ALT_LWFPGASLVS_OFST + 0x8000)

// this is the onchip ram base from the DMA's point of view
#define  DMA_DATA_BASE  FFT_SUB_SGDMA_FROM_FFT_FFT_SUB_DATA_BASE
#define  DMA_RESULT_BASE  (FFT_SUB_SGDMA_FROM_FFT_FFT_SUB_DATA_BASE + 0x8000)

// the FFT csr register from the point of view of the processor
#define FFT_CSR_BASE (ALT_LWFPGASLVS_OFST + FFT_SUB_FFT_STADAPTER_0_BASE)
// this can be 32 - 4K but must be a power of 2
// *****************        feel free to change this value  ****************
// 128 256 512 1024 2048 4096
#define DATA_LENGTH 128

// this is the physical address of lw bridge.
// ALT_LWFPGASLVS_OFST  ->this is the physical address of lw bridge.  defined int socal/socal.h

void const *g_preparser_strings[] = {
        "FILE=" __FILE__,
        "DATE=" __DATE__,
        "TIME=" __TIME__
};

int fft_main(int  waveform) {
    int i;
	unsigned int real, image, wave;
	// need two descriptors 1 - to fft 2- from fft
	sgdma_standard_descriptor descriptor_to, descriptor_from;

	printf("\r\n\r\nHello from SoC FPGA to everyone!\r\n");
	printf ("This program was called with \"%d\".\r\n",waveform);
	//  just parsing the input parameters

	if(waveform==0)
	{
		printf(" **** square wave **** \r\n");
	}
	if(waveform==1)
	{
		printf(" **** sine wave ****\r\n");
	}


	//  load the right wave form into memory and make a copy in a file for display purposes.
	for (i=0;i<DATA_LENGTH;i++)
	{
		if (waveform==0)
			wave=	((i%32)<16)?0x7fff:0x8000; // square wave

		if (waveform==1)
			wave =0xffff&(int)((( sin(2.0*3.1415*(float)i/16.0)*0x7fff))); //sine wave

		// change to big endian
		printf("signal %i %i\r\n",i, (int)(short)wave);
		// real portion in the lower 16 of the 32 bit word.  The lower 16 is the image part which is zero for this example
		//wave = wave<<16;
		alt_write_word((volatile uint32_t)(DATA_BASE + 4*i),wave);
	}

	// zero out the results area  ( notice it is twice a large because the results has 2 32 numbers
	// 1 32 bit number is the read and the other is the imaginary portion)
	for (i=0;i<DATA_LENGTH*2;i++)
	{
		alt_write_word( RESULT_BASE+4*i,0);
	}

	// need to tell the FFT how long the fft is
	alt_write_word (FFT_CSR_BASE,DATA_LENGTH);

	// now do the real work
	// create 2 descriptors
	// first one reads from the onchip sram named DATA and write to the fft
	// the second reads from the fft and write back to onchip memory at a different offset.
	construct_standard_mm_to_st_descriptor(&descriptor_to, (alt_u32 *)DMA_DATA_BASE, DATA_LENGTH*4,DESCRIPTOR_CONTROL_GENERATE_SOP_MASK |DESCRIPTOR_CONTROL_GENERATE_EOP_MASK );
	construct_standard_st_to_mm_descriptor(&descriptor_from, (alt_u32 *)DMA_RESULT_BASE, DATA_LENGTH*8, DESCRIPTOR_CONTROL_END_ON_EOP_MASK );

	// now write the constructors to the DMA
	write_standard_descriptor(SGDMA_TO_FFT_CSR_BASE, SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE, &descriptor_to);
	write_standard_descriptor(SGDMA_FROM_FFT_CSR_BASE, SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE, &descriptor_from);


	// could just poll here for completion
	while( ((unsigned int)read_csr_status (SGDMA_TO_FFT_CSR_BASE) & CSR_DESCRIPTOR_BUFFER_EMPTY_MASK)!=2);
	printf("SGDMA_TO_FFT_CSR status %i\r\n", (int)read_csr_status (SGDMA_TO_FFT_CSR_BASE));
	while( ((unsigned int)read_csr_status (SGDMA_FROM_FFT_CSR_BASE) & CSR_DESCRIPTOR_BUFFER_EMPTY_MASK)!=2);
	printf("SGDMA_FROM_FFT_CSR status %i\r\n", (int)read_csr_status (SGDMA_FROM_FFT_CSR_BASE));

	// now read the results from memory
	printf("\r\n\r\nfft =  real,  image\r\n");
	for(i=0;i<DATA_LENGTH;i++)
	{
		real = alt_read_word(RESULT_BASE + 8*i);
		image = alt_read_word(RESULT_BASE + 8*i + 4);
		printf("fft = %x %i %i\r\n",i, real, image);
	}


	return 0;
}
