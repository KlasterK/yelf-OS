TOP_DIR   := $(CURDIR)
BUILD_DIR := $(TOP_DIR)/build
export TOP_DIR BUILD_DIR

.PHONY: all iso clean noelf yeself qemu

all: noelf yeself iso

noelf:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C noelf

yeself:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C yeself

qemu: $(BUILD_DIR)/primus.iso
	qemu-system-i386 -cdrom $< $(FLAGS)

clean:
	rm -rf $(BUILD_DIR)

iso: $(BUILD_DIR)/primus.iso

$(BUILD_DIR)/primus.iso:
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/*.k $(BUILD_DIR)/iso/

	echo -e "set timeout=5\nset default=0\n\n" > $(BUILD_DIR)/iso/boot/grub/grub.cfg

	for k in $(BUILD_DIR)/*.k; do \
		name=$$(basename $$k .k) ; \
		echo "menuentry \"$$(basename $$k .k)\" {" >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
		echo "    multiboot2 /$$(basename $$k)"    >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
		echo -e "}\n" >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
	done

	grub-mkrescue -o $(BUILD_DIR)/primus.iso $(BUILD_DIR)/iso/
