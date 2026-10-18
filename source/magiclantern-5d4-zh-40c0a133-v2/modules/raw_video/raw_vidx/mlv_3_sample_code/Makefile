BUILD_DIR = build
SRC_DIR = ../../../../src
INCLUDE_ARGS = -I.. -I./include -I$(SRC_DIR)
CFLAGS = -m32 -ggdb3 $(INCLUDE_ARGS)

.PHONY: clean default $(BUILD_DIR)

OBJS = $(BUILD_DIR)/create_mlv

default: $(OBJS)
	touch $(BUILD_DIR)/complete

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	-@rm -rf -- $(BUILD_DIR)

$(BUILD_DIR)/function_overrides.o: ./function_overrides.c | $(BUILD_DIR)
	gcc -c ./function_overrides.c -o $(BUILD_DIR)/function_overrides.o $(CFLAGS)

$(BUILD_DIR)/mlv_3.o: ../mlv_3.c | $(BUILD_DIR)
	gcc -c ../mlv_3.c -o $(BUILD_DIR)/mlv_3.o $(CFLAGS)

$(BUILD_DIR)/create_mlv: create_mlv.c $(BUILD_DIR)/mlv_3.o $(BUILD_DIR)/function_overrides.o ../mlv_3.h | $(BUILD_DIR)
	gcc -o $(BUILD_DIR)/create_mlv create_mlv.c $(BUILD_DIR)/function_overrides.o $(BUILD_DIR)/mlv_3.o $(CFLAGS)
