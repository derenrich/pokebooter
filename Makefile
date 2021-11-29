SOURCES=boot.rom

all: $(SOURCES)

clean:
	-rm *.bin *.o *.elf boot.rom

main.elf:
	gcc -m32 -c -fcf-protection=none -mgeneral-regs-only -mno-red-zone  -mgeneral-regs-only  -fomit-frame-pointer -fno-pic -fno-exceptions   -ffreestanding -Wall -Werror main.c  -o main.elf  -Os

main.o: main.elf
	ld -m elf_i386 -T link.ld -nostdlib main.elf -o main.o
#objcopy -O binary -j .text.startup  main.elf main.o

ff.bin:
	tr '\0' '\377' < /dev/zero | head -c 1024000 > ff.bin

boot.rom: main.o ff.bin
	nasm boot.asm -f bin -o boot.bin
	cat /dev/zero | head -c 32256 > zeros.bin
	cat boot.bin main.o ff.bin  > boot.rom
