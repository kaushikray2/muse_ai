# Makefile for PawPrint Tracker 2nd Gen (Tracker_2026_ra0)
# Renesas RA0E2 (R7FA0E2094CFJ, Cortex-M23) + Renesas FSP, built with arm-none-eabi-gcc
#
# Usage:
#   make            - build build/tracker.elf (+ .hex, .bin, .map, size report)
#   make clean      - remove build artifacts
#   make size       - show section sizes of the last build
#
# Requires: arm-none-eabi-gcc in PATH (tested with 13.2.1)

TARGET   := tracker
BUILD    := build
OBJDIR   := $(BUILD)/obj

# ---------------------------------------------------------------- toolchain
CC       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
SIZE     := arm-none-eabi-size

# ---------------------------------------------------------------- CPU / ABI
# RA0E2 is a Cortex-M23
CPUFLAGS := -mcpu=cortex-m23 -mthumb -mfloat-abi=soft

# ---------------------------------------------------------------- sources
# Application + generated + HAL
APP_SRCS := $(shell find src -name '*.c')
GEN_SRCS := $(shell find ra_gen -name '*.c')

# FSP drivers used by this project
FSP_DRIVERS := r_tau r_rtc_c r_icu r_ioport r_sau_uart r_sau_spi \
               r_flash_lp r_elc r_adc_d r_dtc
FSP_SRCS := $(foreach d,$(FSP_DRIVERS),$(wildcard ra/fsp/src/$(d)/*.c))

# FSP BSP: common + RA0E2 + CMSIS device startup
BSP_SRCS := ra/fsp/src/bsp/mcu/all/bsp_common.c \
            ra/fsp/src/bsp/mcu/all/bsp_clocks.c \
            ra/fsp/src/bsp/mcu/all/bsp_delay.c \
            ra/fsp/src/bsp/mcu/all/bsp_group_irq.c \
            ra/fsp/src/bsp/mcu/all/bsp_guard.c \
            ra/fsp/src/bsp/mcu/all/bsp_io.c \
            ra/fsp/src/bsp/mcu/all/bsp_irq.c \
            ra/fsp/src/bsp/mcu/all/bsp_register_protection.c \
            ra/fsp/src/bsp/mcu/all/bsp_sbrk.c \
            ra/fsp/src/bsp/mcu/ra0e2/bsp_linker.c \
            ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c \
            ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c

# SX126x LoRa driver
SX126X_SRCS := $(shell find sx126x_driver/src -maxdepth 1 -name '*.c')

SRCS := $(APP_SRCS) $(GEN_SRCS) $(FSP_SRCS) $(BSP_SRCS) $(SX126X_SRCS)
OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)
DEPS := $(OBJS:.o=.d)

# ---------------------------------------------------------------- includes
INCLUDES := -I. \
            -Ira/fsp/inc \
            -Ira/fsp/inc/api \
            -Ira/fsp/inc/instances \
            -Ira/fsp/src/bsp/cmsis/Device/RENESAS/Include \
            -Ira/arm/CMSIS_6/CMSIS/Core/Include \
            -Ira_cfg/fsp_cfg \
            -Ira_cfg/fsp_cfg/bsp \
            -Ira_gen \
            -Isrc \
            -Isrc/app \
            -Isrc/hal \
            -Isrc/service \
            -Isrc/test \
            -Isx126x_driver/src

# ---------------------------------------------------------------- flags
CFLAGS := $(CPUFLAGS) \
          -std=gnu11 \
          -Og -g3 \
          -ffunction-sections -fdata-sections \
          -fno-builtin \
          -Wall -Wextra -Wno-unused-parameter \
          -MMD -MP \
          $(INCLUDES)

LDFLAGS := $(CPUFLAGS) \
           -T script/fsp.lld \
           -L . \
           -Wl,--gc-sections \
           -Wl,-Map=$(BUILD)/$(TARGET).map \
           -Wl,--start-group -lc -lm -lgcc -Wl,--end-group \
           -specs=nosys.specs

# ---------------------------------------------------------------- rules
.PHONY: all clean size

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin size

$(BUILD)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(BUILD)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

size: $(BUILD)/$(TARGET).elf
	$(SIZE) -B $<

clean:
	rm -rf $(BUILD)

-include $(DEPS)
