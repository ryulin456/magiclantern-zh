# define the module name - make sure name is max 8 characters
MODULE_NAME = vid_af
MODULE_OBJS = $(BUILD_DIR)/vid_af.o
# Keep this module loadable on the user-verified 5D3 r9 core, which does not
# export the newer menu_snprintf helper used by the global translated build.
MODULE_CFLAGS += -DVIDAF_LEGACY_R9
TOP_DIR = ../..

# include modules environment
include $(TOP_DIR)/modules/Makefile
.DEFAULT_GOAL := $(BUILD_DIR)/module_complete
