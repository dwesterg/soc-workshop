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
#include "hps.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "csr_regs.h"
#include "sgdma_dispatcher.h"
#include "descriptor_regs.h"
#include <sys/mman.h>
#include <math.h>
#include "string.h"

#define MEMTEST_CSR_STATUS_REG                           (0x0)
#define MEMTEST_CSR_BIT_FAILURE_REG   	                 (0x4)
#define MEMTEST_CSR_SIZE_REG                             (0x8)
#define MEMTEST_CSR_CONTROL_REG                          (0xC)
#define MEMTEST_CSR_PRBS_INIT_0_REG                      (0x10)
#define MEMTEST_CSR_PRBS_INIT_1_REG                      (0x14)
#define MEMTEST_CSR_PRBS_INIT_2_REG                      (0x18)
#define MEMTEST_CSR_PRBS_INIT_3_REG                      (0x1C)
#define MEMTEST_CSR_FAILURE_0_REG   	                 (0x20)
#define MEMTEST_CSR_FAILURE_1_REG   	                 (0x24)
#define MEMTEST_CSR_FAILURE_2_REG   	                 (0x28)
#define MEMTEST_CSR_FAILURE_3_REG   	                 (0x2c)

// read/write macros for each 32 bit register of the CSR port
#define MEMTEST_RD_CSR_STATUS(base)                    *(unsigned long *)(base + MEMTEST_CSR_STATUS_REG)
#define MEMTEST_RD_CSR_CONTROL(base)                   *(unsigned long *)(base + MEMTEST_CSR_CONTROL_REG)
#define MEMTEST_RD_CSR_BIT_FAILURE_REG(base)           *(unsigned long *)(base + MEMTEST_CSR_BIT_FAILURE_REG)
#define MEMTEST_RD_CSR_FAILURE_0_REG(base)             *(unsigned long *)(base + MEMTEST_CSR_FAILURE_0_REG)
#define MEMTEST_RD_CSR_FAILURE_1_REG(base)             *(unsigned long *)(base + MEMTEST_CSR_FAILURE_1_REG)
#define MEMTEST_RD_CSR_FAILURE_2_REG(base)             *(unsigned long *)(base + MEMTEST_CSR_FAILURE_2_REG)
#define MEMTEST_RD_CSR_FAILURE_3_REG(base)             *(unsigned long *)(base + MEMTEST_CSR_FAILURE_3_REG)

#define MEMTEST_WR_CSR_SIZE(base, data)                *(unsigned long *)(base + MEMTEST_CSR_SIZE_REG) = data
#define MEMTEST_WR_CSR_CONTROL(base, data)             *(unsigned long *)(base + MEMTEST_CSR_CONTROL_REG) = data
	
#define MEMTEST_WR_CSR_PRBS_INIT_0(base, data)         *(unsigned long *)(base + MEMTEST_CSR_PRBS_INIT_0_REG) = data
#define MEMTEST_WR_CSR_PRBS_INIT_1(base, data)         *(unsigned long *)(base + MEMTEST_CSR_PRBS_INIT_1_REG) = data
#define MEMTEST_WR_CSR_PRBS_INIT_2(base, data)         *(unsigned long *)(base + MEMTEST_CSR_PRBS_INIT_2_REG) = data
#define MEMTEST_WR_CSR_PRBS_INIT_3(base, data)         *(unsigned long *)(base + MEMTEST_CSR_PRBS_INIT_3_REG) = data

/*
#define SGDMA_TO_FFT_CSR_BASE ((int)MAPPED_BASE + READ_MSGDMA_DISP_CSR_BASE)
#define SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE ((int)MAPPED_BASE + READ_MSGDMA_DISP_DESCRIPTOR_SLAVE_BASE)
#define SGDMA_FROM_FFT_CSR_BASE ((int)MAPPED_BASE + WRITE_MSGDMA_DISP_CSR_BASE)
#define SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE ((int)MAPPED_BASE + WRITE_MSGDMA_DISP_DESCRIPTOR_SLAVE_BASE)
#define DATA_BASE ((int)MAPPED_BASE + ONCHIP_MEMORY2_0_BASE )
#define DATA_SIZE ONCHIP_MEMORY2_0_SPAN
#define RESULT_BASE (DATA_BASE + (DATA_SPAN/2))
#define DATA_LENGTH 128
                                                                             
#define MEMTEST_BASE ((int)MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE)
*/

// this is the physical address of lw bridge.
//#define BASE_ADDRESS 0xff200000

// ALT_LWFPGASLVS_OFST



int main(int argc, char** argv) {

	void *MAPPED_BASE;	// where linux see the lw bridge.
	unsigned int *data;
	int i, ii;       
	int mem;
	
	sgdma_standard_descriptor descriptor0, descriptor1;
	
	printf("\n\nHello from SoC FPGA to everyone!\n");
	
	// we need to get a pointer to the LW_BRIDGE from the softwares point of view. 	
	
	// need to open a file.
	/* Open /dev/mem */
	printf("Openning /dev/mem\n");
	if ((mem = open ("/dev/mem", O_RDWR | O_SYNC)) == -1)
		fprintf(stderr, "Cannot open /dev/mem\n"), exit(1);
	// now map it into lw bridge space
	MAPPED_BASE = mmap (0, 0x90000, PROT_READ|PROT_WRITE, MAP_SHARED, mem, ALT_LWFPGASLVS_OFST);
	
	if(MAPPED_BASE == (void *) -1) {
		printf("Memory map failed. error %i\n", (int)MAPPED_BASE);
		perror("mmap");
	}

	printf("Clearing Data mem\n");
	data = (unsigned int *) ((int) MAPPED_BASE + ONCHIP_MEMORY2_0_BASE);
	// now that the fpga space is mapped we need to clear out the onchip ram so it is ready for data
	for (i=0;i<ONCHIP_MEMORY2_0_SPAN/4;i++)
	{                                                                               
		data[i] = i ;
	}
		

	//DMA from LFSR generator to MEM
	MEMTEST_WR_CSR_CONTROL((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0x0);
	MEMTEST_WR_CSR_PRBS_INIT_0((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0xa5a55a5a);
	MEMTEST_WR_CSR_PRBS_INIT_1((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0xf0f00f0f);
	MEMTEST_WR_CSR_PRBS_INIT_2((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0x0ff0f0f0);
	MEMTEST_WR_CSR_PRBS_INIT_3((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0x5a5aa5a5);	
	
	
	
	for (ii=0; ii<16;ii++)
	{

		MEMTEST_WR_CSR_SIZE((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 1024);
		
		//Please note, this only works because the sgdma masters and the hps see the onchip ram at the same address
		//you can do this a couple of ways
		//   Dual port mem
		//   connect sgmda masters to f2h bridge
		//   pipeline bridge to offset address
		//   in the address tab, qsys actually allows every master to have its own view of the world
		//	IE: a single slave can have multiple addresses
		
		construct_standard_st_to_mm_descriptor (&descriptor1, (alt_u32 *)(ALT_LWFPGASLVS_OFST + ONCHIP_MEMORY2_0_BASE), 1024, DESCRIPTOR_CONTROL_END_ON_EOP_MASK );
		
		while ((RD_CSR_STATUS((int) MAPPED_BASE + WRITE_MSGDMA_DISP_CSR_BASE) & CSR_DESCRIPTOR_BUFFER_FULL_MASK) != 0) {}  // spin until there is room for another descriptor to be written to the SGDMA
		write_standard_descriptor ((int) MAPPED_BASE + WRITE_MSGDMA_DISP_CSR_BASE, (int) MAPPED_BASE + WRITE_MSGDMA_DISP_DESCRIPTOR_SLAVE_BASE, &descriptor1);
		
		while ((RD_CSR_STATUS((int) MAPPED_BASE + WRITE_MSGDMA_DISP_CSR_BASE) & (CSR_BUSY_MASK | CSR_DESCRIPTOR_BUFFER_EMPTY_MASK)) != CSR_DESCRIPTOR_BUFFER_EMPTY_MASK) {}  // spin until not busy and desc fifo empty
		
		//print some stuff
		for (i=0;i<16;i++)
		{
			printf("data[%d] = 0x%x\n",i, data[i]);
		}
		
		//DMA from mem to LFSR check
		construct_standard_mm_to_st_descriptor (&descriptor0, (alt_u32 *)(ALT_LWFPGASLVS_OFST + ONCHIP_MEMORY2_0_BASE), 1024,DESCRIPTOR_CONTROL_GENERATE_SOP_MASK |DESCRIPTOR_CONTROL_GENERATE_EOP_MASK );
		
		while ((RD_CSR_STATUS((int) MAPPED_BASE + READ_MSGDMA_DISP_CSR_BASE) & CSR_DESCRIPTOR_BUFFER_FULL_MASK) != 0) {}  // spin until there is room for another descriptor to be written to the SGDMA
		write_standard_descriptor ((int) MAPPED_BASE + READ_MSGDMA_DISP_CSR_BASE, (int) MAPPED_BASE + READ_MSGDMA_DISP_DESCRIPTOR_SLAVE_BASE, &descriptor0);
		
		while ((RD_CSR_STATUS((int) MAPPED_BASE + WRITE_MSGDMA_DISP_CSR_BASE) & (CSR_BUSY_MASK | CSR_DESCRIPTOR_BUFFER_EMPTY_MASK)) != CSR_DESCRIPTOR_BUFFER_EMPTY_MASK) {}  // spin until not busy and desc fifo empty

		MEMTEST_WR_CSR_CONTROL((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, 0x0);
		MEMTEST_WR_CSR_PRBS_INIT_0((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, data[4]);
		MEMTEST_WR_CSR_PRBS_INIT_1((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, data[5]);
		MEMTEST_WR_CSR_PRBS_INIT_2((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, data[6]);
		MEMTEST_WR_CSR_PRBS_INIT_3((int) MAPPED_BASE + MEMTEST_MEMTEST_CONTROLLER_BASE, data[7]);	


	}
	
	munmap(MAPPED_BASE,0x90000 );
	close(mem);
	
	return 0;
}
