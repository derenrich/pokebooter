#include<stdint.h>


void putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR)
{
    // VGA color palette https://en.wikipedia.org/wiki/Video_Graphics_Array#/media/File:VGA_palette_with_black_borders.svg
    unsigned char* location = (unsigned char*)0xA0000 + 320 * pos_y + pos_x;
    *location = VGA_COLOR;
}
void write_string(int color, const char *string, uint16_t pos)
{
    volatile char *video = (volatile char*)0xB8000 + pos * 2;
    while(*string != 0 )
    {
        *video =  *string;
        string++;
        video++;
        *video = color;
        video++;
    }
}

void write_hex(int color, uint32_t hex, uint16_t pos)
{
    write_string(color, "0x", pos);
    volatile char *video = (volatile char*)0xB8000 + 4 + pos * 2;
    for(int i = 0; i < 8; i++) {
        uint32_t val = (hex >> ((7 - i) * 4)) & 0xf;
        char c = val + 0x30;
        if (val > 9) {
            c += 7;
        }
        *video =  c;
        video++;
        *video = color;
        video++;
    }
}


void sleep(uint32_t t) {
    for (uint32_t i =0; i < 100000; i++) {
        for (uint32_t j =0; j < 1000; j++) {
            for (int k=0; k < t; k++) {
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
            }
        }
    }
}


static volatile int pos = 0xcafebabe;

extern void isr_test1();
__asm__(".global _isr_test1\n"
        "isr_test1:\n\t"
        "pusha\n\t"
        "call handle\n\t"
        "popa\n\t"
        "ret");


extern void handle_invalid_op();
__asm__(".global handle_invalid_op\n"
        "handle_invalid_op:\n\t"
        //"pusha\n\t"
        //"call handle\n\t"
        //"popa\n\t"
        "iret");


extern void handle_gpf();
__asm__(".global handle_gpf\n"
        "handle_gpf:\n\t"
        //"pusha\n\t"
        //"call handle\n\t"
        //"popa\n\t"
        "iret");

struct interrupt_frame {
    uint64_t instruction_pointer;
    uint64_t code_segment;
    uint64_t rflags;
    uint64_t register_stack_pointer;
    uint64_t stack_segment;
} __attribute__((packed));
//__attribute__((interrupt)) 
void handle(){//struct interrupt_frame* frame) {
    //__asm__("movl $0xdeadbeef, 0x0");
    write_hex(0xf0, pos, 0);
    volatile char *video = (volatile char*)0xB8000 + pos;
    *video = 'X';
    video ++;
    *video = 0xf0;
    //__asm__("hlt");
    //__asm__("cli");
    pos += 2;
    //__asm__("ret");
    return;
}


#include "port.c"
#include "ata.c"
#include "idt.h"

static struct idt_entry idt[256];
static struct idt_ptr idt_pointer;
const char* hellos =  "Hello, World! Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!";

__attribute__ ((section (".text.main"))) void main()  {
  //for (int i = 0; i < 50; i++) {
  //      for (int j = 0; j < 50; j++) {
  //          char color = 0x1f;//0b00000000;
  //          putpixel(100 + i, 100 + j, color);
  //      }
  // }

    //__asm__("cli");
    //__asm__("hlt");
    //while(1);
    //write_hex(0xf0, 0);
    //write_hex(0xf0, (int) &hellos);
    //write_string(0xf0, hellos);
   

    //volatile int cs = 0;
    //__asm__("mov %%cs, %%ax;\n\tmov %%ax, %0" : "=m"(cs));
    //write_hex(0xf0, cs);

    idt_pointer.limit = (sizeof (struct idt_entry) * 256) - 1;
    idt_pointer.base = (unsigned int) idt;
    idt[49].always0 = 0;
    idt[49].base_lo = ((uint32_t) &isr_test1);
    idt[49].base_hi = ((uint32_t) &isr_test1) >> 16;
    idt[49].sel = 0x08; // CS = 0x8
    idt[49].flags = 0b11101110;//0x8e;

    // GP Fault
    idt[0xd].always0 = 0;
    idt[0xd].base_lo = ((uint32_t) &handle_gpf);
    idt[0xd].base_hi = ((uint32_t) &handle_gpf) >> 16;
    idt[0xd].sel = 0x08; // CS = 0x8
    idt[0xd].flags = 0b11101110;//0x8e;

    // GP Fault
    //idt[6].always0 = 0;
    //idt[6].base_lo = ((uint32_t) &handle_invalid_op);
    //idt[6].base_hi = ((uint32_t) &handle_invalid_op) >> 16;
    //idt[6].sel = 0x08; // CS = 0x8
    //idt[6].flags = 0b11101110;//0x8e;


    //write_hex(0xf0, ((uint32_t) &handle));
    //write_hex(0xf0, idt[49].base_lo);
    //write_hex(0xf0, (uint32_t) &idt_pointer);
    
    //__asm__("lidt %0" :: "m"(idt_pointer));
    //__asm__("sti");
    //__asm__("int $49");
    //sleep(5);

    //__asm__("int $49");
    //sleep(5);

    //__asm__("int $49");

    //__asm__("cli");
    //__asm__("hlt");
    //while(1);
    //write_hex(0xf0, *buf, 0);
    write_string(0xf0, "start read", 0);

    
    // read in the game boy emulator
    volatile uint16_t* gb_buf = (void *) 0x01000000;
    read_sectors(gb_buf, 2000, 128);

    // read in the game boy rom
    volatile uint16_t* gb_rom = (void *) 0x0F000000;
    read_sectors(gb_rom, 4000, 64);

    //buf = read_sectors_ATA_PIO(buf, 2, 0);
    //buf = read_sectors_ATA_PIO(buf, 2 + 256, 0);
    //volatile uint32_t* buf2 = (void *) 0x01000000;

    //write_hex(0xf0, *(gb_buf), 0);
    //sleep(10);

    asm volatile ("ljmp $0x08, $0x01000000");
    //asm volatile ("ljmp $0x08, $0x9000");
    //asm volatile ("ljmp $0x01000000");
    //sleep(5);

    //int x = 0;
    //if (x == 0) {
    //    write_string(0xf0, "Hello, World! Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!");//, "Hello, World!");
    //}
    __asm__("cli");
    __asm__("hlt");
    while(1);

}
