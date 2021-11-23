
load_c:
	mov bx, 0x8000	; address to read to
	mov al, 0x80		; read AL sectors
	mov dl, [BOOT_DRIVE]	; drive number
	mov cl, 0x02	; start from sector 2
	mov ch, 0x00	; cylinder 0
	mov dh, 0x00	; head 0
	call disk_load
	ret

disk_load:
	push ax         ; Store AX on stack so later we can recall
					; how many sectors were request to be read ,
					; even if it is altered in the meantime
	call check_lba_mode
	mov ah, 0x02	; BIOS read sector function
	int 0x13        ; BIOS interrupt
	jc disk_error   ; Jump if error ( i.e. carry flag set )
	pop dx          ; Restore AX from the stack to DX
	cmp dl, al      ; if AL ( sectors read ) != DL ( sectors expected )
	jne disk_error  ; display error message
	ret

	disk_error:
		mov si,DISK_ERROR_MSG
		call print_string
		call print_hex
		jmp $
		hlt


check_lba_mode:
	pusha
	mov ah, 0x41
	mov BX, 0x55AA
	mov dl, 0x80
	int 0x13
	jc no_lba_mode
	popa
	ret
	no_lba_mode:
		mov si,DISK_ERROR_MSG
		call print_string
		jmp $
		hlt
		cli


disk_load_lba:
	mov si, lba_packet
	mov ah, 0x42
	mov dl, [BOOT_DRIVE]
	int 0x13
	jc lba_disk_error
	call memory_dump
	ret
	lba_disk_error:
		mov si, DISK_ERROR_MSG
		call print_string
		call print_hex
		jmp $

memory_dump:
	mov bx, 0
	dump_loop:
	mov ax, [0x8000 + bx]
	call print_hex
	inc bx
	cmp bx, 508
	jne dump_loop
	ret

; Variables
DISK_ERROR_MSG db "Disk read error: ", 0
align 4
; see https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)#LBA_in_Extended_Mode
lba_packet:
	db 0x10	; packet length
	db 0	; zero
	.block_num dw 4	; sectors to transfer 
	.buffer dw 0x8000, 0x0
	.lba_start_low dd 1 ; linear address (starts at 0)
	.lba_start_high dd 0


