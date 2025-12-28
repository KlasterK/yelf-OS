include $(TOP_DIR)/include.mk

TARGET := $(BUILD_DIR)/noelf.k
SOURCE := main.s

.PHONY: all

all: $(TARGET)

$(TARGET): $(SOURCE)
	nasm -f bin $< -o $@
	$(call check_multiboot2,$@)
