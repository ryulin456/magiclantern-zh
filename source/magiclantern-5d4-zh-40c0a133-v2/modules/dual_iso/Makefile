# define the module name - make sure name is max 8 characters
MODULE_NAME = dual_iso
# Preserve the upstream implementation in source. The 5D4 artifact must not
# link sensor-write callbacks or CMOS addresses from other cameras.
MODULE_OBJS = $(BUILD_DIR)/safe_5d4.o
TOP_DIR = ../..

# include modules environment
include $(TOP_DIR)/modules/Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/module_complete

# include cr2hdr makefile rules
include Makefile.cr2hdr
