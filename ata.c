#include<stdint.h>

#define STATUS_BSY (0b10000000)
#define STATUS_RDY (0b01000000)
#define STATUS_DF  (0b00100000)
#define STATUS_DRQ (0b00001000)
#define STATUS_ERR (0b00000001)

#define READ_SECTOR_COMMAND 0x20

#define MASTER_SELECT (0xE0)
#define BASE_PORT 0x1F0
#define DATA_REGISTER (BASE_PORT + 0)
#define ERR_REGISTER (BASE_PORT + 1)
#define SECTOR_COUNT_REGISTER (BASE_PORT + 2)
#define LBA_LOW_REGISTER (BASE_PORT + 3)
#define LBA_MID_REGISTER (BASE_PORT + 4)
#define LBA_HIGH_REGISTER (BASE_PORT + 5)
#define DRIVE_REGISTER (BASE_PORT + 6)
#define COMMAND_REGISTER (BASE_PORT + 7)

#define DEVICE_CONTROL_REGISTER (0x3F6)

//This is really specific to our OS now, assuming ATA bus 0 master 
//Source - OsDev wiki
static void ATA_wait_BSY();
static void ATA_wait_DRQ();
static void read_status_wait();
static void ATA_wait_ERR() ;
static void reset_ATA() ;


volatile uint16_t* read_sectors_ATA_PIO(volatile uint16_t* target, uint32_t LBA, uint8_t sector_count)
{
	reset_ATA();
	read_status_wait();
	ATA_wait_BSY();
	ATA_wait_ERR();

	port_byte_out(DRIVE_REGISTER, MASTER_SELECT | ((LBA >>24) & 0xF));
	port_byte_out(ERR_REGISTER, 0);
	port_byte_out(SECTOR_COUNT_REGISTER, sector_count);
	port_byte_out(LBA_LOW_REGISTER, (uint8_t) LBA);
	port_byte_out(LBA_MID_REGISTER, (uint8_t)(LBA >> 8));
	port_byte_out(LBA_HIGH_REGISTER, (uint8_t)(LBA >> 16)); 
	// make sure things are ok before sending command
	read_status_wait();
	ATA_wait_BSY();
	ATA_wait_ERR();
	port_byte_out(COMMAND_REGISTER, READ_SECTOR_COMMAND); // Send the read command

	read_status_wait();
	int sector_count_val = (sector_count == 0) ? 256 : sector_count;
	for (int j =0; j < sector_count_val; j++)
	{
		ATA_wait_BSY();

		ATA_wait_ERR();

		ATA_wait_DRQ();

		for(int i=0; i < 256; i++) {
			target[i] = port_word_in(DATA_REGISTER);
			// DEBUG prints
			// write_string(0xf0, "copying....    ", 50);
			// write_hex(0xf0, target[i], 20);
			// write_hex(0xf0, i, 0);
			// sleep(1);
		}
		// wait 400ns after done
		read_status_wait();

		target+=256;
	}
	return target;
}

static void reset_ATA() 
{
	port_byte_out(DEVICE_CONTROL_REGISTER, 4);
	port_byte_out(DEVICE_CONTROL_REGISTER, 0);
	for (int i = 0; i < 4; i++) {
		// wait 400ns
		port_byte_in(DEVICE_CONTROL_REGISTER);
	}
}
static void read_status_wait() {
	for (int i = 0; i < 14; i++) {
		// see https://wiki.osdev.org/ATA_PIO#400ns_delays
		port_byte_in(COMMAND_REGISTER);
	}
}

static void ATA_wait_ERR()   //Wait for error to be 0
{
	while((port_byte_in(COMMAND_REGISTER) & 1));
}


static void ATA_wait_BSY()   //Wait for bsy to be 0
{
	char x = 0;
	int count = 0;
	while((x = port_byte_in(COMMAND_REGISTER)) & ( STATUS_BSY)) {
		count ++;
		if (count > 1024) {
		//	break;
		}
	}
}
static void ATA_wait_DRQ()  // Wait fot drq to be 1
{
	while(!(port_byte_in(COMMAND_REGISTER) & STATUS_RDY));
}

// convenience wrapper
volatile uint16_t*  read_sectors(volatile uint16_t* target, uint32_t LBA, int sectors) {

	int start_sector = LBA;
	int end_sector = LBA + sectors;
	while (start_sector + 256 < end_sector) {
		// Debug prints
		//write_hex(0xf0, start_sector, 0);
		//write_string(0xf0, "read at....    ", 50);
		//sleep(2);
		// 0 means read 256 sectors of 512 bytes each = 128KiB
		target = read_sectors_ATA_PIO(target, start_sector, 0);
		start_sector += 256;
	}
	return read_sectors_ATA_PIO(target, start_sector, (end_sector - start_sector));
}
