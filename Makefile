SOURCES=combined.rom

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


combined.rom: boot.rom
	# copy over 2000 sectors of data from combined rom
	dd bs=1 obs=1 if=boot.rom of=combined.rom count=1024000
	# after that copy the gb emulator at sector 2000 onwards
	dd bs=1 obs=1 if=chester_gb/chester.bin of=combined.rom seek=1024000 count=1024000
	# after that copy the gb rom at sector 4000 onwards
	dd bs=1 obs=1 if=pokemon.gb of=combined.rom seek=2048000

