#include "chester.h"
#include "cpu.h"
#include "gpu.h"
#include "interrupts.h"
#include "keys.h"
#include "mmu.h"
#include "loader.h"
#include "logger.h"
#include "save.h"
#include "sync.h"
#include "timer.h"

#define NULL (0)


void write_string_c(int color, const char *string, uint16_t pos)
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

void write_hex_c(int color, uint32_t hex, uint16_t pos)
{
    write_string_c(color, "0x", pos);
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

void delay_c(uint32_t ms)
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


bool init(chester *chester, uint8_t* rom)
{

  chester->rom = NULL;
  chester->bootloader = NULL;

  chester->keys_cumulative_ticks = 0;
  chester->keys_ticks = 15000;

  chester->save_timer = 0;
  chester->save_game_file = NULL;
  chester->save_supported = false;

  uint32_t rom_size = 0;

  chester->rom = rom;
  rom_size = 1048576;

  if (!chester->rom)
    {
      return false;
    }
    write_string_c(0xf0, "init", 0);
    write_hex_c(0xf0, (int) &cpu_reset, 30);
  delay_c(4000);


  cpu_reset(&chester->cpu_reg);




  if (!gpu_init(&chester->g, chester->gpu_init_cb))
    {
      return false;
    }

  mmu_reset(&chester->mem);

  const mbc type = get_type(chester->rom);

  if (type == NOT_SUPPORTED)
    {
      return false;
    }

  mmu_set_rom(&chester->mem, chester->rom, type, rom_size);

#ifdef CGB
  switch (chester->rom[0x0143])
  {
  case 0x80:
  case 0xC0:
    chester->mem.cgb_mode = true;
    break;
  }
#endif

  mmu_set_keys(&chester->mem, &chester->k);
  keys_reset(&chester->k);

  sync_init(&chester->s, 100000, chester->ticks_cb);

  chester->save_supported = false;

  return true;
}

void register_keys_callback(chester *chester, keys_cb cb)
{
  chester->k_cb = cb;
}

void register_get_ticks_callback(chester *chester, get_ticks_cb cb)
{
  chester->ticks_cb = cb;
}

void register_delay_callback(chester *chester, delay_cb cb)
{
  chester->delay_cb = cb;
}

void register_gpu_init_callback(chester *chester, gpu_init_cb cb)
{
  chester->gpu_init_cb = cb;
}

void register_gpu_uninit_callback(chester *chester, gpu_uninit_cb cb)
{
  chester->gpu_uninit_cb = cb;
}

void register_gpu_alloc_image_buffer_callback(chester *chester, gpu_alloc_image_buffer_cb cb)
{
  chester->gpu_alloc_image_buffer_cb = cb;
}

void register_gpu_render_callback(chester *chester, gpu_render_cb cb)
{
  chester->gpu_render_cb = cb;
}

void register_serial_callback(chester *chester, serial_cb cb)
{
  chester->mem.serial_cb = cb;
}

void uninit(chester *chester)
{
  // NEVER  UNINIT
}


#if CGB
bool get_color_correction(chester *chester)
{
  return chester->g.color_correction;
}

void set_color_correction(chester *chester, bool color_correction)
{
  chester->g.color_correction = color_correction;
}
#endif

int run(chester *chester)
{
  int run_cycles = 4194304 / 4;
  while(run_cycles > 0)
    {
      if (chester->bootloader && !chester->mem.bootloader_running)
        {
          mmu_set_bootloader(&chester->mem, NULL);

	  // just leak
          //free(chester->bootloader);
          chester->bootloader = NULL;

          cpu_reset(&chester->cpu_reg);
        }

      cpu_debug_print(&chester->cpu_reg, ALL);
      mmu_debug_print(&chester->mem, ALL);
      gpu_debug_print(&chester->g, ALL);

      if (cpu_next_command(&chester->cpu_reg, &chester->mem))
        {
          gb_log (ERROR, "Could not process any longer");
          cpu_debug_print(&chester->cpu_reg, ERROR);
          mmu_debug_print(&chester->mem, ERROR);
          return -1;
        }

      // STOP should be handled
      if (gpu_update(&chester->g, &chester->mem, chester->cpu_reg.clock.last.t, chester->gpu_render_cb, chester->gpu_alloc_image_buffer_cb))
        {
          gb_log (ERROR, "GPU error");
          gpu_debug_print(&chester->g, ERROR);
          return -2;
        }

      if (!chester->cpu_reg.stop)
        timer_update(&chester->cpu_reg, &chester->mem);

      if (chester->keys_cumulative_ticks > chester->keys_ticks)
        {

          chester->keys_cumulative_ticks = 0;

          switch(chester->k_cb(&chester->k))
            {
            case -1:
              return 1;
            case 1:
              isr_set_if_flag(&chester->mem, MEM_IF_PIN_FLAG);
              chester->cpu_reg.halt =  false;
              chester->cpu_reg.stop =  false;
              break;
            default:
              break;
            }
        }
      else
        {
          chester->keys_cumulative_ticks += chester->cpu_reg.clock.last.t;
        }

      sync_time(&chester->s, chester->cpu_reg.clock.last.t, chester->ticks_cb, chester->delay_cb);

      run_cycles -= chester->cpu_reg.clock.last.t;
    }

  return 0;
}
