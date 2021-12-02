SOURCES=chester.bin

all: $(SOURCES)

clean:
	-rm *.bin *.o *.elf *.rom

chester.elf:
	gcc -m32 -c -fcf-protection=none  -mno-red-zone   -fomit-frame-pointer -fno-pic -fno-exceptions   -ffreestanding -Wall -Werror *.c   -Os

chester.bin: chester.elf ff.bin
	rm -f chester.bin
	ld -m elf_i386 -T link.ld -nostdlib *.o -o chester_start.bin
	cat chester_start.bin ff.bin > chester.bin
ff.bin:
	tr '\0' '\377' < /dev/zero | head -c 1024000 > ff.bin

#boot.rom: main.o ff.bin
#	nasm boot.asm -f bin -o boot.bin
#	cat /dev/zero | head -c 32256 > zeros.bin
#	cat boot.bin main.o ff.bin  > boot.rom
