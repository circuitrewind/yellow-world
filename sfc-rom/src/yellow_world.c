////////////////////////////////////////////////////////////////////////////////
// UTILITY TO FLASH THE "GAME PROCESSOR RAM CASSETTE" USING A SNES/SFC, LOADING
// DATA VIA THE 2P CONTROLLER PORT INTO RAM IN SMALL CHUNKS, AND THEN PUSHING
// THAT INTO THE CART'S SRAM
////////////////////////////////////////////////////////////////////////////////

/*
PINOUT:
5v		orange
clock	yellow
latch	brown
data	red
ground	black
*/



/*
latch protocol:
2 pulses	- good
3 pulses	- retry

single pulse should happen when latch is sent per block to read BOTH controller
ports. so this one happens automatically. this tells the RP2040 to just "continue"

if more than one pulse is received by the RP2040 without a data read, then re-read
that data segement.

segments are 64-bytes in length, follow by X-bytes checksum (X current undefined)
*/


#include <snes.h>
#include <snes/console.h>
#include <string.h>



#define P1 (*(vuint8 *)0x4016)
#define P2 (*(vuint8 *)0x4017)


#define WRITELOCK (*(vuint8 *)0x206000)
#define WRITE_ROM ( (vuint8 *)0xC00000)


extern char tilfont, palfont;
extern u8 txt_pal_adr; 


extern void print_screen_map(	u16 x, u16 y,
								unsigned char *map,
								u8 attributes,
								unsigned char *buffer);

extern void consoleVblank(void);



#define MODE_NONE	0x00
#define MODE_DATA	0x01
#define MODE_CRC	0x02
#define MODE_ERROR	0xFF


u16	rom_block	= 0;
u8	rom_mode	= MODE_NONE;




////////////////////////////////////////////////////////////////////////////////
// CRC16 PRECALCULATED DATA
////////////////////////////////////////////////////////////////////////////////
static const u16 crc_tab16[256] = {
	0x0000,	0xc0c1,	0xc181,	0x0140,
	0xc301,	0x03c0,	0x0280,	0xc241,
	0xc601,	0x06c0,	0x0780,	0xc741,
	0x0500,	0xc5c1,	0xc481,	0x0440,
	0xcc01,	0x0cc0,	0x0d80,	0xcd41,
	0x0f00,	0xcfc1,	0xce81,	0x0e40,
	0x0a00,	0xcac1,	0xcb81,	0x0b40,
	0xc901,	0x09c0,	0x0880,	0xc841,
	0xd801,	0x18c0,	0x1980,	0xd941,
	0x1b00,	0xdbc1,	0xda81,	0x1a40,
	0x1e00,	0xdec1,	0xdf81,	0x1f40,
	0xdd01,	0x1dc0,	0x1c80,	0xdc41,
	0x1400,	0xd4c1,	0xd581,	0x1540,
	0xd701,	0x17c0,	0x1680,	0xd641,
	0xd201,	0x12c0,	0x1380,	0xd341,
	0x1100,	0xd1c1,	0xd081,	0x1040,
	0xf001,	0x30c0,	0x3180,	0xf141,
	0x3300,	0xf3c1,	0xf281,	0x3240,
	0x3600,	0xf6c1,	0xf781,	0x3740,
	0xf501,	0x35c0,	0x3480,	0xf441,
	0x3c00,	0xfcc1,	0xfd81,	0x3d40,
	0xff01,	0x3fc0,	0x3e80,	0xfe41,
	0xfa01,	0x3ac0,	0x3b80,	0xfb41,
	0x3900,	0xf9c1,	0xf881,	0x3840,
	0x2800,	0xe8c1,	0xe981,	0x2940,
	0xeb01,	0x2bc0,	0x2a80,	0xea41,
	0xee01,	0x2ec0,	0x2f80,	0xef41,
	0x2d00,	0xedc1,	0xec81,	0x2c40,
	0xe401,	0x24c0,	0x2580,	0xe541,
	0x2700,	0xe7c1,	0xe681,	0x2640,
	0x2200,	0xe2c1,	0xe381,	0x2340,
	0xe101,	0x21c0,	0x2080,	0xe041,
	0xa001,	0x60c0,	0x6180,	0xa141,
	0x6300,	0xa3c1,	0xa281,	0x6240,
	0x6600,	0xa6c1,	0xa781,	0x6740,
	0xa501,	0x65c0,	0x6480,	0xa441,
	0x6c00,	0xacc1,	0xad81,	0x6d40,
	0xaf01,	0x6fc0,	0x6e80,	0xae41,
	0xaa01,	0x6ac0,	0x6b80,	0xab41,
	0x6900,	0xa9c1,	0xa881,	0x6840,
	0x7800,	0xb8c1,	0xb981,	0x7940,
	0xbb01,	0x7bc0,	0x7a80,	0xba41,
	0xbe01,	0x7ec0,	0x7f80,	0xbf41,
	0x7d00,	0xbdc1,	0xbc81,	0x7c40,
	0xb401,	0x74c0,	0x7580,	0xb541,
	0x7700,	0xb7c1,	0xb681,	0x7640,
	0x7200,	0xb2c1,	0xb381,	0x7340,
	0xb101,	0x71c0,	0x7080,	0xb041,
	0x5000,	0x90c1,	0x9181,	0x5140,
	0x9301,	0x53c0,	0x5280,	0x9241,
	0x9601,	0x56c0,	0x5780,	0x9741,
	0x5500,	0x95c1,	0x9481,	0x5440,
	0x9c01,	0x5cc0,	0x5d80,	0x9d41,
	0x5f00,	0x9fc1,	0x9e81,	0x5e40,
	0x5a00,	0x9ac1,	0x9b81,	0x5b40,
	0x9901,	0x59c0,	0x5880,	0x9841,
	0x8801,	0x48c0,	0x4980,	0x8941,
	0x4b00,	0x8bc1,	0x8a81,	0x4a40,
	0x4e00,	0x8ec1,	0x8f81,	0x4f40,
	0x8d01,	0x4dc0,	0x4c80,	0x8c41,
	0x4400,	0x84c1,	0x8581,	0x4540,
	0x8701,	0x47c0,	0x4680,	0x8641,
	0x8201,	0x42c0,	0x4380,	0x8341,
	0x4100,	0x81c1,	0x8081,	0x4040
};




////////////////////////////////////////////////////////////////////////////////
// CRC16 CALCULATION FUNCTION
////////////////////////////////////////////////////////////////////////////////
static u16 crc_16(const unsigned char *input_str, int num_bytes) {
	u16 crc;
	const unsigned char *ptr;
	int a;

	crc = 0xc0de;
	ptr = input_str;

	for (a=0; a<num_bytes; a++) {
		crc = (crc >> 8) ^ crc_tab16[ (crc ^ (u16) *ptr++) & 0x00FF ];
	}

	return crc;
}




////////////////////////////////////////////////////////////////////////////////
// CONVERT INTEGER OF VARIOUS SIZES AND POINTER TO HEX STRING
////////////////////////////////////////////////////////////////////////////////
void to_str(char *buffer, unsigned long long value, char count) {
	char *input = (char*)&value;
	char val;
	char i;

	for (i=count-1; i>=0; i--) {
		val = (input[i] >> 4) & 0x0F;
		*buffer++ = (val > 0x9) ? (val+'7') : (val+'0');

		val = input[i] & 0x0F;
		*buffer++ = (val > 0x9) ? (val+'7') : (val+'0');
	}

	*buffer = 0x00;
}

inline void u32_to_str(char *buffer, unsigned long long value) {
	to_str(buffer, value, 4);
}

inline void u16_to_str(char *buffer, unsigned int value) {
	to_str(buffer, value, 2);
}

inline void u8_to_str(char *buffer, unsigned char value) {
	to_str(buffer, value, 1);
}

inline void ptr_to_str(char *buffer, const void *value) {
	const unsigned int *hax = (const unsigned int*) &value;
	u16_to_str(buffer+0, hax[1]);
	u16_to_str(buffer+4, hax[0]);
}




////////////////////////////////////////////////////////////////////////////////
// DRAW TEXT ON THE SCREEN
////////////////////////////////////////////////////////////////////////////////
inline void print(u16 x, u16 y, const char *data) {
	print_screen_map(x<<1, y<<1, scr_txt_font_map, txt_pal_adr, (u8*)data);
	scr_txt_dirty = 2;
}




////////////////////////////////////////////////////////////////////////////////
// CLEANUP ROM CODE COPIED INTO RAM, FIXING "JSL" INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////
void copy_rom_to_ram() {
	char buff[32] = "";
	char *ptr = (char*)0x7f8000;
	int i, j;
	for (j=0; j<16; j++) {
		u8_to_str(buff, j);
		print(1, 1, buff);

		for (i=0; i<0x1000; i++) {
			if (ptr[0] == 0x22  &&  ptr[3] == 0x00) {
				ptr[3] = 0x7f;
			}
			ptr++;
		}

	}
}




////////////////////////////////////////////////////////////////////////////////
// OUR "MAIN" FUNCTION ONCE WE'RE OPERATING FULLY IN RAM
////////////////////////////////////////////////////////////////////////////////
void main_ram(void) {
	char buff[32] = "";
	int i;
	int wl, wx, lx;
	int input;
	u16 crc1, crc2;
	unsigned int data, last;

	last = 0xffff;


	while (1) {
		u16_to_str(buff, snes_vblank_count);
		print(1, 1, buff);

		if (rom_mode == MODE_DATA) {

			// SEND 2ND LATCH
			P1 = 1;
			P1 = 0;

			// A VERY HACKY WAY TO MANIPULATE A 24-BIT POINTER
			// BLAME THE 65816 AND TCC FOR NEEDING THIS HACK :P
			vuint8 *rom	 =	WRITE_ROM;
			u16 *hax	 =	(u16*) &rom;
			hax[0]		&=	0x003F;
			hax[0]		|=	(rom_block <<  6);
			hax[1]		 =	(rom_block >> 10) + 0xC0;
			vuint8 *rm2	 =	rom;

			// COPY 64 BYTES OF DATA TO "ROM"
			for (wx=0; wx<64; wx++) {
				input = 0;
				for (i=0; i<8; i++) {
					input |= P2 << i;
				}

				*rom = input;
				rom++;
			}


			// READ IN 16-BIT CRC AND COMPARE TO OUR CRC
			crc1 = 0;
			for (i=0; i<16; i++) {
				crc1 |= P2 << i;
			}

			crc2 = crc_16(rm2, 64);

			// CRC16 DONT MATCH, DON'T ADVANCE BLOCKS
			// INSTEAD, SEND 3RD LATCH
			if (crc1 != crc2) {
				P1 = 1;
				P1 = 0;


			// 0x2000 * 64 = 512KB (MAX SIZE OF "ROM")
			} else if (++rom_block == 0x2000) {
				// RE-LOCK THE "ROM"
				WRITELOCK	= 1;

				// RESET ROM BLOCK BACK TO ZERO
				rom_block	= 0;
				rom_mode	= MODE_NONE;
			}


			u16_to_str(buff, crc1);
			buff[4] = ' ';
			u16_to_str(buff+5, crc2);
			print(1, 14, buff);

			ptr_to_str(buff, rom);
			print(1, 15, buff);

			ptr_to_str(buff, rm2);
			print(1, 16, buff);

			u16_to_str(buff, rom_block);
			print(1, 17, buff);

			consoleVblank();

		}



		// READ P1 CONTROLLER DATA
		// SEND 1ST LATCH
		P1 = 1;
		P1 = 0;
		data = 0;
		for (i=0; i<16; i++) {
			data |= P1 << i;
		}


		// ONLY PRINT NEW TEXT IF IT CHANGES
		if (data != last) {
			u16_to_str(buff, data);
			print(1, 5, buff);
			last = data;


			// B BUTTON PRESSED ON P1
			if (data == 0x0001) {

				// UNLOCK THE WRITE LOCK ON THE "ROM"
				// AND SET MODE TO BEGIN TRANSFER
				if (rom_mode == MODE_NONE) {
					for (wl=0; wl<15; wl++) {
						WRITELOCK = 1;
					}
					rom_mode	= MODE_DATA;
				}
			}

		}


		// UPDATE SCREEN DISPLAY
		scr_txt_dirty = 2;
		consoleVblank();
	}
}




////////////////////////////////////////////////////////////////////////////////
// MAIN FUNCTION IN ROM - THIS IS THE INITIAL ENTRY POINT
////////////////////////////////////////////////////////////////////////////////
int main(void) {
	// Initialize SNES
	consoleInit();

	// Initialize text console with our font
	consoleSetTextVramBGAdr(0x6800);
	consoleSetTextVramAdr(0x3000);
	consoleSetTextOffset(0x0100);
	consoleInitText(0, 16 * 2, &tilfont, &palfont);

	// Init background
	bgSetGfxPtr(0, 0x2000);
	bgSetMapPtr(0, 0x6800, SC_32x32);

	// Now Put in 16 color mode and disable Bgs except current
	setMode(BG_MODE1, 0);
	bgSetDisable(1);
	bgSetDisable(2);

	// Draw a wonderful text :P
	print(5, 10, "Yellow World !");
	print(5, 11, "Testing Some Stuff");

	// Wait for nothing :P
	setScreenOn();


	// SOURCE AND DESTINATION ADDRESSES FOR COPYING ROM TO RAM
	// AND THEN DO THAT COPY, THEN FIX UP "JSL" INSTRUCTIONS
	void *src = (void*)0x008000;
	void *dst = (void*)0x7f8000;
	memcpy(dst, src, 0x7fff);
	copy_rom_to_ram();

	// SET THE ADDRESS TO OUR "MAIN RAM" FUNCTION
	// THERE ARE ISSUES MODIFYING POINTERS AND
	// 32-BIT INTS, SO CONVERT TO 16BIT AND CHANGE
	// THE ONLY 16-BITS THAT NEED CHANGING!
	int *hax	= (int*) &dst;
	hax[0]		= (int) (&main_ram);

	// DISPLAY OUR "MAIN RAM" FUNCTION ADDRESS (DEBUGGING)
	char buff[32] = "";
	ptr_to_str(buff, dst);
	print(1, 3, buff);

	// DISABLE INTERRUPTS, WE DON'T WANT TO JUMP BACK TO ROM
	REG_NMITIMEN = 0x00;


	// CONVERT POINTER TYPES, AND EXECUTE "MAIN RAM" FUNCTION
	void (*ptr)(void);
	ptr = (void*)dst;
	ptr();

	// LOLOLOLOL THIS CODE WILL NEVER BE REACHED
	return 0;
}
