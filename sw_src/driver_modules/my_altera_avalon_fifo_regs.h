/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
******************************************************************************/

#ifndef __ALTERA_AVALON_FIFO_REGS_H__
#define __ALTERA_AVALON_FIFO_REGS_H__

//#include <io.h>

#define ALTERA_AVALON_FIFO_OTHER_INFO_REG                    1
#define ALTERA_AVALON_FIFO_DATA_REG                          0

#define ALTERA_AVALON_FIFO_LEVEL_REG                         0
#define ALTERA_AVALON_FIFO_STATUS_REG                        1
#define ALTERA_AVALON_FIFO_EVENT_REG                         2
#define ALTERA_AVALON_FIFO_IENABLE_REG                       3
#define ALTERA_AVALON_FIFO_ALMOSTFULL_REG                    4
#define ALTERA_AVALON_FIFO_ALMOSTEMPTY_REG                   5

// Read slave
#define IORD_ALTERA_AVALON_FIFO_DATA(base)                   \
        IORD(base, ALTERA_AVALON_FIFO_DATA_REG)
        
#define IORD_ALTERA_AVALON_FIFO_OTHER_INFO(base)             \
        IORD(base, ALTERA_AVALON_FIFO_OTHER_INFO_REG)

// Write slave
#define IOWR_ALTERA_AVALON_FIFO_DATA(base, data)             \
        IOWR(base, ALTERA_AVALON_FIFO_DATA_REG, data)

#define IOWR_ALTERA_AVALON_FIFO_OTHER_INFO(base, data)       \
        IOWR(base, ALTERA_AVALON_FIFO_OTHER_INFO_REG, data)

// Control slave
#define IORD_ALTERA_AVALON_FIFO_LEVEL(base)                  \
        IORD(base, ALTERA_AVALON_FIFO_LEVEL_REG)
        
#define IORD_ALTERA_AVALON_FIFO_STATUS(base)                 \
        IORD(base, ALTERA_AVALON_FIFO_STATUS_REG)
        
#define IORD_ALTERA_AVALON_FIFO_EVENT(base)                  \
        IORD(base, ALTERA_AVALON_FIFO_EVENT_REG)
        
#define IORD_ALTERA_AVALON_FIFO_IENABLE(base)                \
        IORD(base, ALTERA_AVALON_FIFO_IENABLE_REG)
        
#define IORD_ALTERA_AVALON_FIFO_ALMOSTFULL(base)             \
        IORD(base, ALTERA_AVALON_FIFO_ALMOSTFULL_REG)
        
#define IORD_ALTERA_AVALON_FIFO_ALMOSTEMPTY(base)            \
        IORD(base, ALTERA_AVALON_FIFO_ALMOSTEMPTY_REG)
        
#define IOWR_ALTERA_AVALON_FIFO_EVENT(base, data)            \
        IOWR(base, ALTERA_AVALON_FIFO_EVENT_REG, data)
        
#define IOWR_ALTERA_AVALON_FIFO_IENABLE(base, data)          \
        IOWR(base, ALTERA_AVALON_FIFO_IENABLE_REG, data)
        
#define IOWR_ALTERA_AVALON_FIFO_ALMOSTFULL(base, data)       \
        IOWR(base, ALTERA_AVALON_FIFO_ALMOSTFULL_REG, data)
        
#define IOWR_ALTERA_AVALON_FIFO_ALMOSTEMPTY(base, data)      \
        IOWR(base, ALTERA_AVALON_FIFO_ALMOSTEMPTY_REG, data)

#define ALTERA_AVALON_FIFO_EVENT_F_MSK    (0x01)
#define ALTERA_AVALON_FIFO_EVENT_E_MSK    (0x02)
#define ALTERA_AVALON_FIFO_EVENT_AF_MSK   (0x04)
#define ALTERA_AVALON_FIFO_EVENT_AE_MSK   (0x08)
#define ALTERA_AVALON_FIFO_EVENT_OVF_MSK  (0x10)
#define ALTERA_AVALON_FIFO_EVENT_UDF_MSK  (0x20)
#define ALTERA_AVALON_FIFO_EVENT_ALL  (0x3F)

#define ALTERA_AVALON_FIFO_STATUS_F_MSK    (0x01)
#define ALTERA_AVALON_FIFO_STATUS_E_MSK    (0x02)
#define ALTERA_AVALON_FIFO_STATUS_AF_MSK   (0x04)
#define ALTERA_AVALON_FIFO_STATUS_AE_MSK   (0x08)
#define ALTERA_AVALON_FIFO_STATUS_OVF_MSK  (0x10)
#define ALTERA_AVALON_FIFO_STATUS_UDF_MSK  (0x20)
#define ALTERA_AVALON_FIFO_STATUS_ALL  (0x3F)

#define ALTERA_AVALON_FIFO_IENABLE_F_MSK    (0x01)
#define ALTERA_AVALON_FIFO_IENABLE_E_MSK    (0x02)
#define ALTERA_AVALON_FIFO_IENABLE_AF_MSK   (0x04)
#define ALTERA_AVALON_FIFO_IENABLE_AE_MSK   (0x08)
#define ALTERA_AVALON_FIFO_IENABLE_OVF_MSK  (0x10)
#define ALTERA_AVALON_FIFO_IENABLE_UDF_MSK  (0x20)
#define ALTERA_AVALON_FIFO_IENABLE_ALL  (0x3F)

#endif /* __ALTERA_AVALON_FIFO_REGS_H__ */

