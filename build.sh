as -o boot.o boot.s
ld -o boot.bin --oformat binary -e start_ boot.o