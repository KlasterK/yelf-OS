#!/bin/bash

# I could not set up CMake for compiling this, so I'm going to just use a shell script
# Run it from repo root

if test ! -d src; then
    echo "src directory does not exist"
    exit
fi

if test ! -d build; then
    mkdir build
fi

if [[ ! -v CROSS ]]; then
    CROSS="/usr/local/i686-elf"
fi

if [[ ! -v OPT_DBG ]] || [[ "$OPT_DBG" == "g" ]]; then
    NASM_O="$NASM_O -g -F dwarf"
    GCC_O="$GCC_O -Og -g"
    LD_O="$LD_O -g"
fi

$CROSS/bin/i686-elf-g++ $GCC_O -nostdlib -ffreestanding -c src/no_elf_simpliest_os/kernel.cpp -o build/kernel.o
nasm $NASM_O src/no_elf_simpliest_os/boot.s -o build/boot.o -f elf32
nasm $NASM_O src/no_elf_simpliest_os/com.s -o build/com.o -f elf32

$CROSS/bin/i686-elf-ld $LD_O -T src/no_elf_simpliest_os/link.ld build/kernel.o build/boot.o build/com.o -o build/yeself.elf
