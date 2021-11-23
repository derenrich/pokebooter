print_string:
	push ax
print_loop:
	mov al, [si]
	cmp al, 0
	je print_done
	call print_char
	inc si
	jmp print_loop
print_done:
	pop ax
	ret

print_hex:
	pusha
	mov bx, ax
	shr ax, 12
	call print_hex_val
	mov ax, bx
	shr ax, 8
	call print_hex_val
	mov ax, bx
	shr ax, 4
	call print_hex_val
	mov ax, bx	
	call print_hex_val
	mov ax, 32 ; print space
	call print_char
	popa
	ret
print_hex_val:
	and ax, 0xf
	cmp al, 9
	jl decimal
	add al, 7
decimal:	
	add al, 0x30
	call print_char	
	ret
print_char:
	pusha
	mov AH, 0x0E
	mov BH, 0x00
	mov BL, 0x07
	int 0x10
	popa
	ret


	
