TARGET := $(BUILD_DIR)/noelf.k
SOURCE := main.s

.PHONY: all

all: $(TARGET)

$(TARGET): $(SOURCE)
	nasm -f bin $< -o $@
	grub-file --is-x86-multiboot2 $@ \
		&&  echo "$@ is multiboot2 compliant" \
		|| (echo "$@ is not multiboot2 compliant" ; exit 1)
