MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ 0x00000007
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
    align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM
    dd 0, 0, 0, 0, 0
    dd 0    ; Linear frame buffer
    dd 1024 ; Width
    dd 768  ; Height
    dd 32   ; Depth

section .text
extern kmain
global _start

_start:
    cli
    mov esp, stack_top 
    push ebx
    call kmain
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: