#!/bin/bash
set -e

BUILD=build
DISK_IMG=disk.img
STAGE2_SECTORS=16
KERNEL_LBA=20  


mkdir -p $BUILD

echo "[1]compilo stage1 (boot.asm)"
nasm -f bin bootloader/boot.asm -o $BUILD/boot.bin

echo "[2] compilazione del kernel (C + start.asm)"
gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -nostartfiles \
    -O2 -Wall -Wextra -c kernel/kernel.c -o $BUILD/kernel.o
nasm -f elf32 kernel/start.asm -o $BUILD/start.o
ld -m elf_i386 -nostdlib -T kernel/linker.ld -o $BUILD/kernel.elf $BUILD/start.o $BUILD/kernel.o
objcopy -O binary $BUILD/kernel.elf $BUILD/kernel.bin

echo "[3] Calcolo settori kernel"
KSECT=$(( ( $(stat -c%s $BUILD/kernel.bin) + 511 ) / 512 ))
echo " -> Kernel = $KSECT settori (512B)"

echo "[4] compilazione di stage2 (loader.asm)"
nasm -f bin -D KERNEL_LBA=$KERNEL_LBA -D KERNEL_SECTORS=$KSECT bootloader/loader.asm -o $BUILD/loader.bin

echo "[5] creazione del disco"

TOTAL_SECTORS=$((1 + STAGE2_SECTORS + KSECT))
TOTAL_SIZE=$((TOTAL_SECTORS * 512))

truncate -s $TOTAL_SIZE $DISK_IMG

dd if=$BUILD/boot.bin   of=$DISK_IMG bs=512 seek=0           conv=notrunc status=none
dd if=$BUILD/loader.bin of=$DISK_IMG bs=512 seek=1           conv=notrunc status=none
dd if=$BUILD/kernel.bin of=$DISK_IMG bs=512 seek=$KERNEL_LBA conv=notrunc status=none

REAL_SIZE=$(stat -c %s $DISK_IMG)
truncate -s $REAL_SIZE $DISK_IMG

echo "[6] Avvio con QEMU"
qemu-system-i386 -hda disk.img -audiodev pa,id=speaker -machine pcspk-audiodev=speaker -net nic,model=rtl8139 -net user -m 512 