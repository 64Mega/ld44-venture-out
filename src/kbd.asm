;; Again with the keyboard code, but this time trying to get it working with NASM

%ifndef GFKBD_ASM
%define GFKBD_ASM

%include "src\util.asm"

SEGMENT _DATA PUBLIC ALIGN=4 USE32 class=DATA
    GLOBAL _kbd_scanbuffer
    _kbd_scanbuffer resb 128

    old_int_seg resw 1
    old_int_off resd 1

    ThisCodeSegment resw 1


SEGMENT _TEXT PUBLIC ALIGN=4 USE32 class=CODE

GROUP DGROUP _DATA

GLOBAL _kbd_install
_kbd_install: FUNCTION
    push eax
    push ebx
    push ds
    push es

    ;; Save pointer to old ISR
    mov eax, 0x3509
    int 0x21
    mov word [old_int_seg], es
    mov dword [old_int_off], ebx

    ;; Install new ISR
    mov eax, 0x2509
    mov edx, kbd_handler
    mov bx, _TEXT
    mov word [ThisCodeSegment], bx
    mov ds, bx
    int 0x21

    pop es
    pop ds
    pop ebx
    pop eax
ENDFUNCTION

GLOBAL _kbd_uninstall
_kbd_uninstall: FUNCTION
    push eax
    push ebx
    push edx
    push ds

    ;; Restore BIOS keyboard handler state
    cli
    mov ebx, 0x041C
    mov al, byte [ebx]
    mov ebx, 0x041A
    mov byte[ebx], al
    sti

    mov eax, 0x2509
    mov edx, dword [old_int_off]
    mov bx, word [old_int_seg]
    mov ds, bx
    int 0x21

    pop ds
    pop edx
    pop ebx
    pop eax
ENDFUNCTION

;; ISR 
kbd_handler:
    pushfd
    push eax
    push ebx
    push ecx
    push edx
    push ds

    mov ax, DGROUP
    mov ds, ax

    xor eax, eax
    xor ebx, ebx

    ;; Read key
    in al, 0x60

    ;; Is it a key press?
    cmp al, 128
    jnb .key_release
.key_press:
    mov byte [_kbd_scanbuffer+eax], 1
    jmp .key_end
.key_release:
    and al, 127
    mov byte [_kbd_scanbuffer+eax], 0
.key_end:

    pop ds
    pop edx
    pop ecx
    pop ebx
    pop eax

    ;; Handing over to BIOS for debugging reasons
    sub esp, 8
    push ds
    push eax
    mov ax, DGROUP
    mov ds, ax
    mov eax, dword [old_int_off]
    mov [esp+8], eax
    movzx eax, word [old_int_seg]
    mov [esp+12], eax
    pop eax
    pop ds

    iretd

%endif