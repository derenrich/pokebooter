#include "chester.h"
#include <stdbool.h>
#include<stddef.h>
#include "util.h"


unsigned char port_byte_in (unsigned short port) {
    unsigned char result;
    __asm__ volatile("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}
void port_byte_out (unsigned short port, unsigned char data) {
    __asm__ volatile("out %%al, %%dx" : : "a" (data), "d" (port));
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

void putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR)
{
    // VGA color palette https://en.wikipedia.org/wiki/Video_Graphics_Array#/media/File:VGA_palette_with_black_borders.svg
    unsigned char* location = (unsigned char*)0xA0000 + 320 * pos_y + pos_x;
    *location = VGA_COLOR;
}


static volatile int ticks = 0;


uint32_t get_ticks(void)
{
  ticks++;
  return ticks;
}

void delay(uint32_t ms)
{
    for (uint32_t i =0; i < 100; i++) {
        for (uint32_t j =0; j < 1000; j++) {
            for (int k=0; k < ms; k++) {
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
                // run it faster to be less annoying
                //__asm__("nop");
                //__asm__("nop");
                //__asm__("nop");
            }
        }
    }
}


char enable_kb = 0;
char e0_mode = 0;
char up_arrow = 0;
char down_arrow = 0;
char left_arrow = 0;
char right_arrow = 0;
char x = 0;
char z = 0;
char enter = 0;


void read_kb() {
  // enable KB
  int ps2_status = 0x64; //command register
  int ps2_data = 0x60;  // data register

  unsigned char status = port_byte_in(ps2_status); 
  int read_happened = 0;
  while((status & 1) == 1) {
        //write_string(0xf0, "readkb", 0);
      read_happened = 1;
      unsigned char code = port_byte_in(ps2_data);
      switch(code) {
          case 0xE0:
            e0_mode = 1;
            break;
          case 0x1c:
            enter = 1;
            e0_mode = 0;
            break;
          case 0x2d:
            x = 1;
            e0_mode =0;
            break;
          case 0xAD:
            x=0;
            e0_mode = 0;
            break;
          case 0x9c:
            enter = 0;
            e0_mode = 0;
            break;
          case 0x2c :
            z = 1;
            e0_mode = 0;
            break;
          case 0xAC :
            z = 0;
            e0_mode = 0;
            break;
          // down arrow
            case 0x50:
              if (e0_mode) {
                e0_mode = 0;
                down_arrow = 1;
              }
              break;
            case 0xd0:
              if (e0_mode) {
                e0_mode = 0;
                down_arrow = 0;
              }
              break;
          // left arrow
            case 0x4b:
              if (e0_mode) {
                e0_mode = 0;
                left_arrow = 1;
              }
              break;
            case 0xcb:
              if (e0_mode) {
                e0_mode = 0;
                left_arrow = 0;
              }
              break;
          // right arrow
            case 0x4d:
              if (e0_mode) {
                e0_mode = 0;
                right_arrow = 1;
              }
              break;
            case 0xcd:
              if (e0_mode) {
                e0_mode = 0;
                right_arrow = 0;
              }
              break;
          // up arrow
          case 0xc8:
            if (e0_mode) {
              up_arrow = 0;
              e0_mode = 0;
            }
            break;
          case 0x48:
            if (e0_mode) {
              up_arrow = 1;
              e0_mode = 0;
            }
            break;
          default:
            e0_mode = 0;
        }
      //write_hex(0xf0, code, 40);
      //delay(400);
      status = port_byte_in(ps2_status); 
  }
  e0_mode = 0;
  if (!read_happened) {
  //              write_string(0xf0, "noreadkb", 0);
  }
}


int keys_update(keys *k) {
  // TODO: handle keys
    read_kb();
  //write_hex(0xf0, enter, 0);

  k->start = enter;
  k->a = x;
  k->b = z;
  k->left = left_arrow;
  k->right = right_arrow;
  k->down = down_arrow;
  k->up = up_arrow;

  return 1;  
}


bool init_graphics(gpu *g)
{
  //delay(2000);

  // maybe we can do nothing?
  g->pixel_data = 0;
  delay(2000);

  return true;
}

void uninit_graphics(gpu *g)
{
  // this never happens
}

bool lock_texture(gpu *g)
{
          write_string(0xf0, "lock start", 0);
  //delay(2000);

  g->pixel_data = (char *) 0xb000000;
  write_string(0xf0, "lock end", 0);
  //delay(2000);

  return true;
}


static const int Y_OFFSET = 20;
static const int X_OFFSET = 40;


__attribute__ ((noinline))
volatile void render(gpu *g)
{
      write_string(0xf0, "render start", 0);
  //delay(2000);

  const unsigned int per_line_offset = 256 * 4;
  //int last_color = 0;
  for (int y = 0; y < Y_RES; y++) {
    const unsigned int line_offset = per_line_offset * y;
    for (int x = 0; x < X_RES; x ++ ) {
      int color = ( (unsigned char*)g->pixel_data)[x*4 + line_offset];
      
      color = color >> 4;
      color = 0x10 | color;

      putpixel(x + X_OFFSET, y + Y_OFFSET, color);
    }
  }
}




__attribute__ ((section (".text.main")))
int main()
{
  //while(1) {
   read_kb();
  //}
    static chester chester;

    write_string(0xf0, "start", 0);
  delay(2000);

  //write_hex(0xf0, (int) &chester, 40);
  chester.cpu_reg.pc = 0;
  delay(2000);

  register_keys_callback(&chester, &keys_update);
  register_get_ticks_callback(&chester, &get_ticks);
  register_delay_callback(&chester, &delay);
  register_gpu_init_callback(&chester, &init_graphics);
    write_string(0xf0, "gogogo", 0);
  delay(2000);

  register_gpu_uninit_callback(&chester, &uninit_graphics);
  register_gpu_alloc_image_buffer_callback(&chester, &lock_texture);
  register_gpu_render_callback(&chester, &render);
  register_serial_callback(&chester, NULL);

  uint8_t * rom = (uint8_t *) 0x0F000000;

  write_string(0xf0, "registered", 0);
  //write_hex(0xf0, (int) &init, 30);
  delay(2000);

  if (!init(&chester, rom))
    {
      return 2;
    }

  while (1)
    {
      run(&chester);
    }


  write_string(0xf0, "wall", 0);
  //write_hex(0xf0, (int) &init, 30);
  delay(2000);

  __asm__("cli");
  __asm__("hlt");
  while(1);

  return 0;
}
