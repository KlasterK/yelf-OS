TARGET        := $(BUILD_DIR)/yeself.k
SOURCES       := boot.s com.s kernel.cpp
LINKER_SCRIPT := link.ld

NASM      := nasm
NASMFLAGS := -f elf32

CXX      := clang++
CXXFLAGS := -target i686-pc-none-elf \
            -march=i486 \
            -ffreestanding \
            -fno-stack-protector \
            -fno-exceptions \
            -fno-rtti \
            -m32 \
            -std=c++23 \
            -nostdlibinc

LD      := ld.lld
LDFLAGS := -m elf_i386 -static -nostdlib -T $(LINKER_SCRIPT)

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS  += -g -O0 -fno-omit-frame-pointer
    NASMFLAGS += -g -F dwarf
else
    CXXFLAGS  += -O3 -fomit-frame-pointer -flto
    LDFLAGS   += -s -flto
endif

# *.s --> build/*.o
OBJS := $(addprefix $(BUILD_DIR)/, $(patsubst %.s,%.o,$(filter %.s,$(SOURCES))))
# *.cpp --> build/*.o
OBJS += $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(filter %.cpp,$(SOURCES))))

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
	grub-file --is-x86-multiboot2 $@ \
		&&  echo "$@ is multiboot2 compliant" \
		|| (echo "$@ is not multiboot2 compliant" ; exit 1)

$(BUILD_DIR)/%.o: %.s
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
