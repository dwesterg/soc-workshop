/*
 * Copyright (C) 2015 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "my_altera_avalon_fifo_regs.h"

/*
 * demo driver hardware map
 */
#define ROM_OFST	(0)
#define ROM_SPAN	(1024)
#define RAM_OFST	(ROM_OFST + ROM_SPAN)
#define RAM_SPAN	(1024)
#define TIMER_OFST	(RAM_OFST + RAM_SPAN)

/*
 * misc values
 */
#define IO_BUF_SIZE	(8)

/*
 * ioctl values
 *
#define IOC_SET_INTERVAL      (0x4004EE00)
#define IOC_GET_INTERVAL      (0x8004EE00)
#define IOC_GET_MAX_DELAY     (0x8004EE01)
#define IOC_GET_MIN_DELAY     (0x8004EE02)
 */
#define OUR_IOC_TYPE	(0xEE)
#define IOC_SET_INTERVAL	_IOW(OUR_IOC_TYPE, 0, int)
#define IOC_GET_INTERVAL	_IOR(OUR_IOC_TYPE, 0, int)
#define IOC_GET_MAX_DELAY	_IOR(OUR_IOC_TYPE, 1, int)
#define IOC_GET_MIN_DELAY	_IOR(OUR_IOC_TYPE, 2, int)

/*
 * msgdma driver values
 */
#define DMA_DEVICE_BUFFER_SIZE		(1024 * 1024)
#define DMA_DEVICE_MIN_BLOCK_SIZE	(1024)
#define DMA_DEVICE_MIN_BLOCK_MASK	(DMA_DEVICE_MIN_BLOCK_SIZE - 1)

#define CSR_STATUS_REG			(ALTERA_MSGDMA_CSR_STATUS_REG << 2)
#define CSR_CONTROL_REG			(ALTERA_MSGDMA_CSR_CONTROL_REG << 2)
#define CSR_DESCRIPTOR_FILL_LEVEL_REG	(ALTERA_MSGDMA_CSR_DESCRIPTOR_FILL_LEVEL_REG << 2)
#define CSR_RESPONSE_FILL_LEVEL_REG	(ALTERA_MSGDMA_CSR_RESPONSE_FILL_LEVEL_REG << 2)

#define DESC_READ_ADDRESS_REG		(ALTERA_MSGDMA_DESCRIPTOR_READ_ADDRESS_REG << 2)
#define DESC_WRITE_ADDRESS_REG		(ALTERA_MSGDMA_DESCRIPTOR_WRITE_ADDRESS_REG << 2)
#define DESC_LENGTH_REG			(ALTERA_MSGDMA_DESCRIPTOR_LENGTH_REG << 2)
#define DESC_CONTROL_REG		(ALTERA_MSGDMA_DESCRIPTOR_CONTROL_STANDARD_REG << 2)

#define START_DMA_MASK	( \
		ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GO_MASK | \
		ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)

/*
 * fifo driver values
 */
#define FIFO_DATA_REG		(ALTERA_AVALON_FIFO_DATA_REG << 2)

#define FIFO_LEVEL_REG		(ALTERA_AVALON_FIFO_LEVEL_REG << 2)
#define FIFO_STATUS_REG		(ALTERA_AVALON_FIFO_STATUS_REG << 2)
#define FIFO_EVENT_REG		(ALTERA_AVALON_FIFO_EVENT_REG << 2)
#define FIFO_IENABLE_REG	(ALTERA_AVALON_FIFO_IENABLE_REG << 2)
#define FIFO_ALMOSTFULL_REG	(ALTERA_AVALON_FIFO_ALMOSTFULL_REG << 2)
#define FIFO_ALMOSTEMPTY_REG	(ALTERA_AVALON_FIFO_ALMOSTEMPTY_REG << 2)

#define FIFO_MAX_FILL_LEVEL	(256)
