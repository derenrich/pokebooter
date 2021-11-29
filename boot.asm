[BITS 16]
[ORG 0x7C00]

%include "memory_map.asm"

start_:
	jmp 0h:canonicalize 	; force us to CS = 0
canonicalize:
	mov [BOOT_DRIVE], dl	; remember our boot drive
	mov bp, REAL_STACK 		; setup stack
	mov sp, REAL_STACK   
	;;  clear screen 
	mov ah, 07h
	mov al, 0
	mov bh, 07h
	mov cx, 0
	mov dh, 18h
	mov dl, 4fh
	int 10h
	;; print welcome message
	mov si, welcome
	call print_string
	call disk_init


	; setup video mode
	mov ah, 0x00
	mov al, 0x03
	int 0x10


	; load compiled C code to RAM
	call disk_load_lba

	;; enter protected mode
	cli
	xor ax, ax
	mov ds, ax
	lgdt [gdt_desc]
	;lidt [idt_descriptor]
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp 08h:clear_pipe

BOOT_DRIVE: db 0
welcome db "Hello", 0

disk_init:
	mov ah, 0h
	xor dl, dl
	int 13h
	ret

%include "print.asm"
%include "disk_load.asm"
	
[BITS 32]
clear_pipe:
	mov ax, 10h
	mov ds, ax
	mov ss, ax
	mov esp, 090000h
    mov byte [ds:0B8000h], 'P'
    mov byte [ds:0B8001h], 1Bh
	jmp DISK_COPY_LOC


;idt_descriptor:
;    dw 0 ; Size of our idt, always one less than the actual size
;    dd 0 ; Start address of our idt

gdt:
gdt_null:
	dq 0
gdt_code:
	dw 0FFFFh
	dw 0
	db 0
	db 10011010b
	db 11001111b
	db 0
gdt_data:
	dw 0FFFFh
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0
gdt_end:
gdt_desc:
	dw gdt_end - gdt - 1
	dd gdt
	
times 510 - ($ - $$) db 0
dw 0xAA55

