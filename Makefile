TOP_DIR   := $(CURDIR)
BUILD_DIR := $(TOP_DIR)/build
export TOP_DIR BUILD_DIR

.PHONY: all iso clean rebuild noelf yeself qemu qemu-486 bldrun

all: noelf yeself iso

noelf:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C noelf

yeself:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C yeself

qemu: $(BUILD_DIR)/disk.image
	qemu-system-i386 \
		-cdrom $(BUILD_DIR)/primus.iso \
		-serial stdio \
		-hda $(BUILD_DIR)/disk.image \
		$(F) $(FLAGS) $(EXTRA_FLAGS) $(QEMUFLAGS)

qemu-486: $(BUILD_DIR)/disk.image
	qemu-system-i386 \
		-cdrom $(BUILD_DIR)/primus.iso \
		-serial stdio \
		-hda $(BUILD_DIR)/disk.image \
		-cpu    486 \
		-m      16 \
		-icount shift=7,align=off,sleep=on \
		-accel  tcg,thread=single \
		$(F) $(FLAGS) $(EXTRA_FLAGS) $(QEMUFLAGS)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

$(BUILD_DIR)/disk.image:
	dd if=/dev/zero of=$@ bs=1M count=4
	echo "Hello World!" | dd of=$@ bs=1 count=512 conv=notrunc

bldrun: all qemu

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
