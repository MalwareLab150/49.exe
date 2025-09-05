BITS 16
ORG 0x8000

%define KERNEL_TMP_OFF   0x0000
%define KERNEL_TMP_SEG   0x1000
%define KERNEL_LOAD_DEST 0x00100000
%define KERNEL_ENTRY     0x00100000
%ifndef KERNEL_LBA
%define KERNEL_LBA 17
%endif

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 32
%endif
extern_msg db "[STAGE2] Lettura kernel...",0
ok_msg     db " OK\r\n",0
err_msg    db "\r\nErrore lettura kernel",0
pm_msg     db "[STAGE2] Switch a protected mode...",0


ALIGN 8
GDT:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
GDT_END:

GDT_DESC:
    dw GDT_END - GDT - 1
    dd GDT

CODE_SEL equ 0x08
DATA_SEL equ 0x10

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, extern_msg
    call print_str

    mov word [dap+2], KERNEL_SECTORS
    mov word [dap+4], KERNEL_TMP_OFF
    mov word [dap+6], KERNEL_TMP_SEG
    mov dword [dap+8], KERNEL_LBA
    mov dword [dap+12], 0

    mov si, dap
    mov ah, 0x42
    int 0x13
    jc .disk_fail

    mov si, ok_msg
    call print_str

    in  al, 0x92
    or  al, 0000_0010b
    out 0x92, al

    mov si, pm_msg
    call print_str

    lgdt [GDT_DESC]
    cli
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp CODE_SEL:pm_entry

.disk_fail:
    mov si, err_msg
    call print_str
.halt:
    hlt
    jmp .halt

print_str:
    pusha
.next:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    jmp .next
.done:
    popa
    ret

align 16
DAP_SIZE   equ 16
dap:
    db DAP_SIZE
    db 0
    dw 0
    dw 0
    dw 0
    dd 0
    dd 0

BITS 32
pm_entry:
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x00900000

    mov esi, 0x00010000
    mov edi, KERNEL_LOAD_DEST
    mov ecx, KERNEL_SECTORS
    imul ecx, ecx, 512
    shr ecx, 2
    rep movsd

    jmp KERNEL_ENTRY
