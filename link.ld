MEMORY
{
  bootrom (rx): ORIGIN = 0x0, LENGTH = 0x00004000
  boot1  (rx)  : ORIGIN = 0x9000, LENGTH = 0x00004000
  ram    (rw) : ORIGIN = 0xD000, LENGTH = 0x00001000
}


SECTIONS
{
 
	.text.startup  :
	{
		*(.text.startup)
	} > boot1 AT > bootrom

	.text.main  :
	{
		*(.text.main)
	} > boot1 AT > bootrom


    .text :
    {
        *(.text)
    } > boot1 AT > bootrom

    .data ALIGN(0) :  { /* without align it will all be off by 2 bytes */
        *(.data)
    } > boot1  AT > bootrom

    .rodata : {
        *(.rodata)
        *(.rodata.str1.1)
    } > boot1 AT > bootrom

    .bss : {
        *(.bss)
    } > ram AT > bootrom




    /DISCARD/ : {
        *(COMMON)
        *(.comment)
        *(.eh_frame)        
    }

}

OUTPUT_FORMAT(binary)


