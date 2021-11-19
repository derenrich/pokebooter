load_c:
	mov bx, 0x8000	; address to read to
	mov al, 1		; read AL sectors
	mov dl, [BOOT_DRIVE]	; drive number
	mov cl, 0x02	; start from sector 2
	mov ch, 0x00	; cylinder 0
	mov dh, 0x00	; head 0
	call disk_load
	ret

disk_load:
	push ax         ; Store DX on stack so later we can recall
					; how many sectors were request to be read ,
					; even if it is altered in the meantime
	mov ah, 0x02	; BIOS read sector function
	;mov al, dh		; Read DH sectors
	;mov ch, 0x00	; Select cylinder 0
	;mov dh, 0x00   ; Select head 0
	;mov cl, 0x02   ; Start reading from second sector ( i.e.
					; after the boot sector )
	;mov dl, 0x80
	int 0x13        ; BIOS interrupt
	jc disk_error   ; Jump if error ( i.e. carry flag set )
	pop dx          ; Restore DX from the stack
	cmp dl, al     ; if AL ( sectors read ) != DL ( sectors expected )
	jne disk_error  ; display error message
	ret

	disk_error:
		mov si,DISK_ERROR_MSG
		call print_string
		call print_hex
		jmp $

	; Variables
	DISK_ERROR_MSG db "Disk read error: ", 0

