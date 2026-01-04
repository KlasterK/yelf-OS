include $(TOP_DIR)/include.mk

TARGET        := $(BUILD_DIR)/yeself.k
LINKER_SCRIPT := link.ld

SOURCES := boot.s kernel.cpp comio.cpp vgaio.cpp fileops.cpp kbdio.cpp
SOURCES += filewrappers.cpp

NASM      := nasm
NASMFLAGS := -f elf32

CXX      := clang++
CXXFLAGS := -target i686-pc-none-elf
CXXFLAGS += -march=i486
CXXFLAGS += -ffreestanding
CXXFLAGS += -fno-stack-protector
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-rtti
CXXFLAGS += -m32
CXXFLAGS += -std=c++23
CXXFLAGS += -masm=intel
CXXFLAGS += -Wall
CXXFLAGS += -Wextra

LD      := clang++
LDFLAGS := -m32
LDFLAGS += -fuse-ld=lld
LDFLAGS += -static
LDFLAGS += -nostdlib
LDFLAGS += -flto
LDFLAGS += -Wl,-T,$(LINKER_SCRIPT)
LDFLAGS += -Wl,-m,elf_i386

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
	$(call check_multiboot2,$@)

$(BUILD_DIR)/%.o: %.s
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
