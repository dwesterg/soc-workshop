#
# Copyright Altera 2013,2014
# All Rights Reserved.
#
QSYS_HDR := hps_0.h
QSYS_HDR_DIR := qsys_headers

SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
HWLIBS_ROOT = $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib
SOCAL_ROOT := $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib/include/socal
HWLIBS_SRC  := 

EXAMPLE_SRC := fft.c hello.c io.c 
EXAMPLE_SRC += msgdma/sgdma_dispatcher.c
C_SRC       := $(EXAMPLE_SRC) $(HWLIBS_SRC)

LINKER_SCRIPT := cycloneV-dk-ram-modified.ld
#LINKER_SCRIPT := cycloneV-dk-ram.ld

MULTILIBFLAGS := -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=neon
CFLAGS  := -g -O0 -Wall -Werror -std=c99 $(MULTILIBFLAGS) -I$(HWLIBS_ROOT)/include -I. -Imsgdma -Iqsys_headers -I$(SOCAL_ROOT)
LDFLAGS := -T$(LINKER_SCRIPT) $(MULTILIBFLAGS)

CROSS_COMPILE := arm-altera-eabi-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)g++
NM := $(CROSS_COMPILE)nm
OC := $(CROSS_COMPILE)objcopy
OD := $(CROSS_COMPILE)objdump
MKIMAGE := $(SOCEDS_ROOT)/host_tools/altera/mkimage/mkimage
RM := rm -rf
CP := cp -f

ELF ?= $(basename $(firstword $(C_SRC))).axf
OBJ := $(patsubst %.c,%.o,$(C_SRC))
BIN = $(basename $(firstword $(C_SRC))).bin
IMG = $(basename $(firstword $(C_SRC)))-mkimage.bin

.PHONY: all
all: $(BIN) $(ELF) $(IMG) $(SPL) 

.PHONY: clean
clean:
	$(RM) $(ELF) $(HWLIBS_SRC) $(OBJ) $(BIN) $(IMG) $(QSYS_HDR_DIR)
	rm -f *.map
	rm -f *.objdump
	rm -f u-boot.scr

%.c: $(HWLIBS_ROOT)/src/hwmgr/%.c
	$(CP) $< $@

$(OBJ): %.o: %.c Makefile $(QSYS_HDR_DIR)/$(QSYS_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $@
	$(NM) $@ > $@.map
	$(OD) -d $@ > $@.objdump
	
$(BIN): $(ELF)
	$(OC) -O binary $(ELF) $(BIN)
		
$(IMG): $(BIN)
	$(MKIMAGE) -A arm -T standalone -C none -a 0x100040 -e 0x100040 -n fft_image -d $(BIN) $(IMG)
	$(MKIMAGE) -A arm -T kernel -O linux -C none -a 0x100040 -e 0x100040 -n baremetal_image -d $(BIN) $(IMG).img
	$(MKIMAGE)  -A arm -O linux -T script -C none -a 0 -e 0 -n My_script -d u-boot.txt u-boot.scr
#to run after uboot 
#fatload mmc_spi 0 0x100040 fft.bin
# go 0x100080
# or 
# this does not work
#fatload mmc_spi 0 0x180000 fft-mkimage.bin
#fatload mmc_spi 0 0x200000 fft.axf
#bootm 0x180000
