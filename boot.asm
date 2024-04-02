extern main
global start
extern gdtdesc

%define SEG_KCODE (1 << 3)
%define SEG_KDATA (2 << 3)





section .text
bits 32
start:
    lgdt [gdtdesc]
    jmp SEG_KCODE:reload_cs

reload_cs:

    ; load 0 into all data segment registers
    mov ax, SEG_KDATA
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    
    mov esp, stack + 4096
    call main
    hlt


section .bss
align 4096

stack:
    resb 4096; Reserve this many bytes