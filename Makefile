include ../micropython-1.29.0/py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

CC = clang-18
SDK = /opt/aros-build/bin/raspi-aarch64/AROS/Developer/

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I$(SDK)include
INC += -idirafter $(SDK)include/aros/posixc
INC += -idirafter $(SDK)include/aros/stdc

UNAME_S := $(shell uname -s)

CFLAGS += --target=aarch64-aros --sysroot=$(SDK)
CFLAGS += $(INC) -Wall -std=c99 $(COPT)
CFLAGS += -fno-pic -fno-pie -mcmodel=large -D__AROS__ -D__linux__ $(CFLAGS_EXTRA)

LD = $(CC)
LDFLAGS_EXTRA = --target=aarch64-aros --sysroot=$(SDK) -z muldefs
LDFLAGS_ARCH = -Wl,-Map=$@.map
LDFLAGS += $(LDFLAGS_MOD) $(LDFLAGS_ARCH) -lm $(LDFLAGS_EXTRA)

CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
CFLAGS += -g  # always include debug info in the ELF
ifeq ($(DEBUG), 1)
CFLAGS += -O0
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

LIBS =

SRC_C = \
	main.c \
	uart_core.c \
	aros_mphal.c \
	gccollect.c \
	rpi.c \
	bcm283x_gpio.c \
	machine_pin.c \
	i2c.c \
	machine_i2c.c \
	spi.c \
	machine_spi.c \
	mini_uart.c \
	pwm.c \
	machine_pwm.c \

SHARED_SRC_C += $(addprefix shared/,\
	libc/printf.c \
	readline/readline.c \
	runtime/pyexec.c \
	runtime/gchelper_generic.c \
	runtime/stdout_helpers.c \
	runtime/sys_stdio_mphal.c \
	$(SHARED_SRC_C_EXTRA) \
	)

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_S:.s=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(DRIVERS_SRC_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C)

all: $(BUILD)/micropython

$(BUILD)/micropython: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

include $(TOP)/py/mkrules.mk
