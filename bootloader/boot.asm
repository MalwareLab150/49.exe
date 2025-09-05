BITS 16
ORG 0x7C00

%define STAGE2_LOAD_SEG 0x0000
%define STAGE2_LOAD_OFF 0x8000
%define STAGE2_SECTORS  16
%define STAGE2_LBA      1

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, msg_boot
    call print_str

    mov word [dap+2], STAGE2_SECTORS
    mov word [dap+4], STAGE2_LOAD_OFF
    mov word [dap+6], STAGE2_LOAD_SEG
    mov dword [dap+8], STAGE2_LBA
    mov dword [dap+12], 0

    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error

    jmp STAGE2_LOAD_SEG:STAGE2_LOAD_OFF

print_str:
    pusha
.next:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .next
.done:
    popa
    ret

disk_error:
    mov si, msg_disk
    call print_str
hang:
    hlt
    jmp hang

msg_boot db "Loading stage2...",0
msg_disk db "Disk error (AH=42h)",0

boot_drive db 0

dap:
    db 0x10
    db 0
    dw 0
    dw 0
    dw 0
    dd 0
    dd 0

TIMES 510-($-$$) db 0
DW 0xAA55
