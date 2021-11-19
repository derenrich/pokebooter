MEMORY
{
  bootrom (rx): ORIGIN = 0x0, LENGTH = 0x00004000
  boot1  (rx)  : ORIGIN = 0x8000, LENGTH = 0x00004000
  ram    (rw) : ORIGIN = 0xD000, LENGTH = 0x00001000  
}


SECTIONS
{
 
	.text.startup  :
	{
		*(.text.startup)
	} > boot1 AT > bootrom

    .text :
    {
        *(.text)
    } > boot1 AT > bootrom

    .data : {
        *(.data)
    } > boot1  


    /DISCARD/ : {
        *(COMMON)
        *(.comment)
        *(.eh_frame)        
    }

}

OUTPUT_FORMAT(binary)


