#include "chester.h"
#include <stdbool.h>
#include<stddef.h>
#include "util.h"


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

int keys_update(keys *k) {
  // TODO: handle keys
  return 0;
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
                __asm__("nop");
                __asm__("nop");
                __asm__("nop");
            }
        }
    }
}

bool init_graphics(gpu *g)
{
  delay(2000);

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



__attribute__ ((noinline))
volatile void render(gpu *g)
{
      write_string(0xf0, "render start", 0);
  //delay(2000);

  const unsigned int per_line_offset = 256 * 4;
  
  for (int y = 0; y < Y_RES; y++) {
    const unsigned int line_offset = per_line_offset * y;
    for (int x = 0; x < X_RES; x ++ ) {
      int color = ( (unsigned char*)g->pixel_data)[x*4 + line_offset];
      putpixel(x, y, color);
    }
  }

        write_string(0xf0, "render end", 0);
//  delay(2000);

}



__attribute__ ((section (".text.main")))
int main()
{
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
