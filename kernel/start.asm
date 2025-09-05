BITS 32
GLOBAL _start
EXTERN kmain ;defined in the kernel.c

SECTION .text
_start:

    mov esp, 0x00800000

    call kmain

.hang:
    hlt
    jmp .hang
