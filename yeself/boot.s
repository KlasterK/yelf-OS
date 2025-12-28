bits 32

%define M_TAG align 8, db 0
%define VIDEO_MEM 0xB8000

; Format of char style:
; 0bABBBCCCD
; where A       - whether char is blinking,
;       B and C - background and foreground colors in 3-bit RGB format,
;       D       - whether char is light
%define DEFAULT_CHAR_STYLE 0b10001110


section .data
    sz_fmt db `Hello %s Wo%cld!\n`, 0
    sz_insertion db `COM`, 0


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
    ; type = framebuffer tag
    dw 5
    ; flags
    dw 0
    ; size
    dd 20
    ; width of screen
    dd 80
    ; height of screen
    dd 25
    ; bit depth, 0 means text mode
    dd 0

M_TAG
    ; type = end tag
    dw 0
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 8
m_end:
