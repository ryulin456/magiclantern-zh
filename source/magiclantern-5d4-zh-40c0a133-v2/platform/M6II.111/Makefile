MODEL = M6II
FW_VERSION = 111

TOP_DIR = ../..

AUTOEXEC_BASE = 0x800000
FIR_BASE = 0x800120

#CANON_FIR  = M6200111.FIR
FIRMWARE_ID = 0x00000811
MAIN_FIRMWARE_ADDR = 0xE0040000

# user_mem_start from cstart struct. Align to 0x10 up
RESTARTSTART = 0xb8d20 # 0xb0078 in ROM

# Cortex A9, binaries loaded as Thumb
CFLAGS_PLATFORM = -mthumb -mlong-calls
PLATFORM_ARCH = armv7-a

ML_BOOT_OBJ = boot-d678.o
ML_SRC_EXTRA_OBJS += $(BUILD_DIR)/function_overrides.o

include ../Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/cam_complete
