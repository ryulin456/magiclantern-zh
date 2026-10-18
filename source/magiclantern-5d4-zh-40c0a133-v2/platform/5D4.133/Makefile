MODEL = 5D4
FW_VERSION = 133

# No official 5D4 release ZIP exists.  Keep the source identity in the camera
# UI instead of manufacturing a date-based release name.
VERSION = dev-40c0a133-derived.5D4.133.zh-CN.r1
BUILD_ID = 40c0a133-derived+5D4-safe+zh-r1
BUILD_USER = ryulin456@gmail.com

# The module Makefiles in this dedicated tree enforce 5D4 safety even when
# modules are built directly, before the platform target is invoked.

TOP_DIR = ../..

AUTOEXEC_BASE = 0x800000
FIR_BASE = 0x800120

#CANON_FIR  = 5D400133.FIR
#UPDATE_NAME_FIR = DUMP_5D4.FIR
MAIN_FIRMWARE_ADDR = 0xFE0A0000

# RESTARTSTART is the location in mem that ML binary will be copied to.
# This should be user_mem_start from cstart struct, aligned up to 0x10.
RESTARTSTART    = 0x1cc400

# Cortex R4
CFLAGS_PLATFORM += -mthumb -mlong-calls
PLATFORM_ARCH = armv7-r

ML_BOOT_OBJ     = boot-d678.o
ML_SRC_EXTRA_OBJS += $(BUILD_DIR)/function_overrides.o

include ../Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/cam_complete
