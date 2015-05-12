#*
# * Copyright (c) 2014, Altera Corporation <www.altera.com>
# * All rights reserved.
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions are met:
# * - Redistributions of source code must retain the above copyright
# *   notice, this list of conditions and the following disclaimer.
# * - Redistributions in binary form must reproduce the above copyright
# *   notice, this list of conditions and the following disclaimer in the
# *   documentation and/or other materials provided with the distribution.
# * - Neither the name of the Altera Corporation nor the
# *   names of its contributors may be used to endorse or promote products
# *   derived from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
# * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# *

# set SOPCINFO to the directory that contains the sopcinfo
SOPCINFO := ./soc_system.sopcinfo

QSYS_HDR := hps_0.h
QSYS_HDR_DIR := qsys_headers

SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
SOCAL_ROOT_14_1 := $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib/include/socal
SOCAL_ROOT_15_0 := $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av/socal

C_SRC := memtest.c msgdma/sgdma_dispatcher.c
CFLAGS := -g -O0 -Werror -Wall -lm -Imsgdma -I. -Iqsys_headers -I$(SOCAL_ROOT_14_1) -I$(SOCAL_ROOT_15_0) -MD

CROSS_COMPILE := arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc
NM := $(CROSS_COMPILE)nm

ifeq ($(or $(COMSPEC),$(ComSpec)),)
RM := rm -rf
else
RM := cs-rm -rf
endif

ELF ?= $(basename $(firstword $(C_SRC)))
OBJ := $(patsubst %.c,%.o,$(C_SRC))

.PHONY: all
all:  $(ELF) $(QSYS_HDR_DIR)/$(QSYS_HDR)

QSYS_HDR := hps_0.h
QSYS_HDR_DIR := qsys_headers

$(QSYS_HDR_DIR)/$(QSYS_HDR): $(SOPCINFO)
	mkdir -p $(QSYS_HDR_DIR)
	sopc-create-header-files $< --output-dir $(QSYS_HDR_DIR)

	
.PHONY:
clean:
	$(RM) $(ELF) $(OBJ) *.objdump *.map *.d msgdma/*.d
	$(RM) $(QSYS_HDR_DIR)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(QSYS_HDR_DIR)/$(QSYS_HDR) $(OBJ)  Makefile
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)
	$(NM) $@ > $@.map
