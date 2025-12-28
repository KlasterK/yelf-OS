# Конфигурация
CROSS      ?= /usr/local/i686-elf
BUILD_DIR  := build
SRC_DIR    := .

# Флаги по умолчанию
NASM_FLAGS := -f elf32
GCC_FLAGS  := -nostdlib -ffreestanding -fno-exceptions -fno-rtti
LD_FLAGS   := -nostdlib

# Отладочные флаги (если OPT_DBG=g или не установлен)
ifneq (,$(filter-out release,$(OPT_DBG)))
    NASM_FLAGS += -g -F dwarf
    GCC_FLAGS  += -Og -g
    LD_FLAGS   += -g
endif

# Цели сборки
OBJS := $(BUILD_DIR)/kernel.o $(BUILD_DIR)/boot.o $(BUILD_DIR)/com.o
TARGET := $(BUILD_DIR)/yeself

# Основная цель
all: $(TARGET)

# Сборка ядра
$(TARGET): $(OBJS) $(SRC_DIR)/link.ld
	$(CROSS)/bin/i686-elf-ld $(LD_FLAGS) -T $(SRC_DIR)/link.ld $(OBJS) -o $@

# Компиляция C++
$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.cpp | $(BUILD_DIR)
	$(CROSS)/bin/i686-elf-g++ $(GCC_FLAGS) -c $< -o $@

# Ассемблирование boot.s
$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.s | $(BUILD_DIR)
	nasm $(NASM_FLAGS) $< -o $@

# Ассемблирование com.s
$(BUILD_DIR)/com.o: $(SRC_DIR)/com.s | $(BUILD_DIR)
	nasm $(NASM_FLAGS) $< -o $@

# Создание директории сборки
$(BUILD_DIR):
	mkdir -p $@

# Очистка
clean:
	rm -rf $(BUILD_DIR)

# Пересборка
rebuild: clean all

# Специальные цели
.PHONY: all clean rebuild check-src
