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

qemu: iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/primus.iso $(F) $(FLAGS) $(EXTRA_FLAGS) $(QEMUFLAGS)

clean:
	rm -rf $(BUILD_DIR)

iso:
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/*.k $(BUILD_DIR)/iso/

	echo -e "set timeout=5\nset default=0\n\n" > $(BUILD_DIR)/iso/boot/grub/grub.cfg

	for k in $(BUILD_DIR)/*.k; do \
		echo "menuentry \"$$(basename $$k .k)\" {" >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
		echo "    multiboot2 /$$(basename $$k)"    >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
		echo -e "}\n" >> $(BUILD_DIR)/iso/boot/grub/grub.cfg ; \
	done

	grub-mkrescue -o $(BUILD_DIR)/primus.iso $(BUILD_DIR)/iso/
