#include "loader.h"


uint8_t* read_file(const char* path, uint32_t *size, const bool is_rom)
{
  char *buffer = 0;
  return (uint8_t*)buffer;
}

mbc get_type(uint8_t* rom)
{
  switch(rom[0x0147])
    {
    case 0x00: // ROM ONLY
      return NONE;
    case 0x01: // MBC1
    case 0x02: // MBC1+RAM
      return MBC1;
    case 0x03: // MBC1+RAM+BATTERY
      return MBC1_BATTERY;
    case 0x05: // MBC2
    case 0x06: // MBC2+BATTERY
    case 0x08: // ROM+RAM
    case 0x09: // ROM+RAM+BATTERY
    case 0x0B: // MMM01
    case 0x0C: // MMM01+RAM
    case 0x0D: // MMM01+RAM+BATTERY
    case 0x0F: // MBC3+TIMER+BATTERY
    case 0x10: // MBC3+TIMER+RAM+BATTERY
      gb_log(WARNING, "Unsupported ROM type");
      return NOT_SUPPORTED;
    case 0x11: // MBC3
    case 0x12: // MBC3+RAM
      return MBC3;
    case 0x13: // MBC3+RAM+BATTERY
      return MBC3_BATTERY;
    case 0x15: // MBC4
    case 0x16: // MBC4+RAM
    case 0x17: // MBC4+RAM+BATTERY
    case 0x19: // MBC5
    case 0x1A: // MBC5+RAM
      return MBC5;
    case 0x1B: // MBC5+RAM+BATTERY
      return MBC5_BATTERY;
    case 0x1C: // MBC5+RUMBLE
    case 0x1D: // MBC5+RUMBLE+RAM
      return MBC5;
    case 0x1E: // MBC5+RUMBLE+RAM+BATTERY
      return MBC5_BATTERY;
    case 0xFC: // POCKET CAMERA
    case 0xFD: // BANDAI TAMA5
    case 0xFE: // HuC3
    case 0xFF: // HuC1+RAM+BATTERY
    default:
      gb_log(WARNING, "Unsupported ROM type");
      return NOT_SUPPORTED;
    }
}
