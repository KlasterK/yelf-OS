bits 32

section .text


global a_com_putc
a_com_putc:
    ; cdecl void (char)
    mov al, byte[esp+4]
    mov dx, 0x3F8
    out dx, al
    ret


global a_com_printf
a_com_printf:
    ; cdecl void (const char*, ...)

    push ebp
    mov ebp, esp

    push esi
    push edi

    ; Get fmt string (1st arg)
    mov esi, dword[ebp+8]
    ; Get pointer to 2nd argument
    lea edi, dword[ebp+12]

    mov dx, 0x3F8

.loop:
    mov al, byte[esi]
    
    test al, al
    jz .end

    cmp al, '%'
    je .prc

    out dx, al

.continue:
    inc ebx
    jmp .loop

.prc:
    inc ebx
    mov al, byte[ebx]

    cmp al, '%'
    je .prc_prc

    cmp al, 's'
    je .prc_s

    jmp .continue

.prc_prc:
    out dx, al
    jmp .continue

.prc_s:
    pop ecx

.prc_s_loop:
    mov al, byte[ecx]

    test al, al
    jz .continue

    out dx, al
    inc ecx
    
    jmp .prc_s_loop

.prc_c:
    pop eax ; cdecl requires all the arguments be at least 4 bytes long
    out dx, al
    jmp .continue

.end:
    pop edi
    pop esi
    pop ebp
    ret