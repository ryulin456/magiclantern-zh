# define the module name - make sure name is max 8 characters
MODULE_NAME = raw_vidx
MODULE_OBJS = $(BUILD_DIR)/raw_vid.o \
              $(BUILD_DIR)/worker.o \
              $(BUILD_DIR)/mlv_3.o \
              $(BUILD_DIR)/event_pusher.o
TOP_DIR = ../../..

# include modules environment
include $(TOP_DIR)/modules/Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/module_complete
