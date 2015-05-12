
C_SRC := demo_devmem.c
CFLAGS := -march=armv7-a -mfloat-abi=hard -mfpu=vfp3 -mthumb-interwork -mthumb -O2 -g -feliminate-unused-debug-types -std=gnu99 -W -Wall -Werror -Wc++-compat -Wwrite-strings -Wstrict-prototypes -pedantic \

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

$(ELF): $(C_SRC)
	$(CC) $(CFLAGS) $< -o $@

