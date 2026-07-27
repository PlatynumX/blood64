V ?= 0
SOURCE_DIR := src
BUILD_DIR := build
FILESYSTEM_DIR := filesystem
PROG_NAME := blood64-r1

include $(N64_INST)/include/n64.mk

CFLAGS += -Iinclude -std=gnu11 -Wall -Wextra -Werror -falign-functions=32
OBJS := \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/blood64_rff.o \
	$(BUILD_DIR)/blood64_art.o \
	$(BUILD_DIR)/blood64_runtime.o

all: $(PROG_NAME).z64

$(PROG_NAME).z64: N64_ROM_TITLE = "Blood64 r1"
$(PROG_NAME).z64: $(BUILD_DIR)/$(PROG_NAME).dfs
$(BUILD_DIR)/$(PROG_NAME).elf: $(OBJS)

$(BUILD_DIR)/$(PROG_NAME).dfs: $(shell find $(FILESYSTEM_DIR) -type f 2>/dev/null)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(OBJS:.o=.d)

clean:
	rm -rf $(BUILD_DIR) *.z64 *.elf

.PHONY: all clean
