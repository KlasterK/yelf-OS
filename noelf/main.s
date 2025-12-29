bits 32

%define M_TAG align 8, db 0
%define VIDEO_MEM 0xB8000

; Format of char style:
; 0b[Do Blink, BG Red, Green, Blue, Intensity, FG Red, Green, Blue]
                           ; bRGBiRGB
%define DEFAULT_CHAR_STYLE 0b00000111

section .data
    stdout_char_n dd 0
data_end:

section .bss
    stack_bottom:
        resb 1024
    stack_top:
bss_end:

section .text
start:
    ; Already in protected mode
    ; All the selectors are pointing to 0

    ; Initing the stack
    mov esp, stack_top
    mov ebp, esp

    ; Save EAX before calling putc
    mov ecx, eax

    mov bx, DEFAULT_CHAR_STYLE << 8 | ':'
    call putc

    cmp ecx, 0x36D76289
    jne .error

    mov bl, ')'
    jmp .end
.error:
    mov bl, '('

.end:
    call putc

    ; Print all the bytes of initial EAX (now ECX) onto the screen
    mov edi, 4
.rot_loop:
    rol ecx, 8
    mov bl, cl
    call putc

    dec edi
    jnz .rot_loop

    ; Hang 
    jmp $

putc:
    ; input: BH = char style, BL = char to put
    ; output: EAX trashed

    mov eax, VIDEO_MEM
    add eax, dword[stdout_char_n]
    mov word[eax], bx
    add dword[stdout_char_n], 2
    ret

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
    dd -1
    ; load end address, or .data end
    dd data_end
    ; .bss end, 0 means no .bss
    dd bss_end

M_TAG
    ; type = entry address tag
    dw 3
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 12
    ; entry address
    dd start

M_TAG
    ; type = end tag
    dw 0
    ; flags
    dw 0
    ; size of tag including type, flags and size fields
    dd 8
m_end:
