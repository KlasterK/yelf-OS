bits 32

%define M_TAG align 8, db 0

section .data
    _dummy db 0


section .bss
    stack_bottom:
        resb 1024
    stack_top:


section .text
global a_start
a_start:
    ; Already in protected mode
    ; All the selectors are pointing to 0

    ; Init the stack
    mov esp, stack_top

    extern c_main
    call c_main

    ; Hang 
    jmp $


section .m align=8
    ; multiboot2 header

m_begin:
    ; magic
    dd 0xE85250D6
    ; arch = x86
    dd 0
    ; header length
    dd m_end - m_begin
    ; checksum (checksum + magic + arch + header length = 0)
    dd 0 - 0xE85250D6 - m_end + m_begin

M_TAG
    ; type = address tag
    dw 2
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 24
    ; multiboot2 header address
    dd m_begin
    ; load address, or .text begin, -1 means image's begin
    extern load_begin
    dd load_begin
    ; load end address, or .data end
    extern load_end
    dd load_end
    ; .bss end, 0 means no .bss
    extern bss_end
    dd bss_end

M_TAG
    ; type = entry address tag
    dw 3
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 12
    ; entry address
    dd a_start

M_TAG
    ; type = end tag
    dw 0
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 8
m_end:
