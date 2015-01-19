
C_SRC := validator_devmem.c
CFLAGS := -g -O0 -Werror -Wall

CROSS_COMPILE := arm-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc

ifeq ($(or $(COMSPEC),$(ComSpec)),)
RM := rm -rf
else
RM := cs-rm -rf
endif

ELF ?= $(basename $(firstword $(C_SRC)))

.PHONY: all
all:  $(ELF)

.PHONY:
clean:
	$(RM) $(OBJ) *.objdump *.map *.d $(ELF)

$(ELF): validator_devmem.c
	$(CC) $(CFLAGS) -c $< -o $@

