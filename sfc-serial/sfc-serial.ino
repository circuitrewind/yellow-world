#include "binary/binary.h"
#include "color/color.h"
#include "led/led_array.h"
#include "cook.h"
#include "crc16.h"

#define IO_PIN  15
#define CLOCK_PIN 14
#define LATCH_PIN 26

#define LED_BUILTIN 16
led_array light(LED_BUILTIN, 1, LED_RGB);


volatile uint8_t  io_mode   = 0;
volatile uint8_t  io_bit    = 0;
volatile uint32_t io_byte   = 0;
volatile uint32_t io_last   = 0;
volatile uint16_t io_ltch   = 0;

volatile uint16_t crc       = 0;

#define MODE_NONE   0x00
#define MODE_DATA   0x01
#define MODE_CRC    0x02
#define MODE_ERROR  0xFF




////////////////////////////////////////////////////////////////////////////////
// IRQ - FIGURE OUT WHAT SEQUENCE WE'RE IN BASED ON NUMBER OF LATCHES
////////////////////////////////////////////////////////////////////////////////
void io_latch() {
	io_ltch++;


	// START OF A 64-BYTE DATA BLOCK
	if (io_ltch == 2) {           
		io_mode = MODE_DATA;

	// INVALID CRC16, RETRY 64-BYTE BLOCK
	} else if (io_ltch == 3) {    
		Serial.print("BAD CRC DETECTED\n");
		io_byte = io_last;
	}

	// IGNORE POLLING CONTROLLER DATA
	uint32_t b = (io_byte & 0x3F);
	if (b >= 1 && b <= 4) {
		io_mode = MODE_NONE;
		io_byte = io_last;
		io_ltch = 0;
	}

	// RESET DATA BIT
	io_bit = 1;
	char data = cook_sfc[io_byte] & 0x01;
	digitalWrite(IO_PIN, !data);
}




////////////////////////////////////////////////////////////////////////////////
// IRQ - TRANSMIT 1 BIT AT A TIME ON THE DATA IO PIN
////////////////////////////////////////////////////////////////////////////////
void io_clock() {
	char data = 0; 
	char incr = 0; 
	
	if (io_mode == MODE_DATA) {
		// SEND CURRENT BIT DOWN THE I/O PIN
		data = (cook_sfc[io_byte] >> (io_bit)) & 0x01;
		digitalWrite(IO_PIN, !data);

		if (++io_bit >= 8) {
			io_bit  = 0;
			io_byte++;

			if (io_byte >= cook_sfc_len) {
				io_byte = 0;
			}

			incr = io_byte & 0x003F;

			if (incr == 0x00) {
				io_mode = MODE_CRC;
				crc     = crc_16(cook_sfc + io_last, 64);

				Serial.print("CRC16: ");
				Serial.print(crc, HEX);
				Serial.print("\n");
			
			} else if (incr == 0x04) {
				io_ltch   = 0;
				io_last   = io_byte & 0xFFFFFFC0;  
			}
		}



	} else if (io_mode == MODE_CRC) {
		// SEND CURRENT BIT DOWN THE I/O PIN
		data = (crc >> io_bit) & 0x01;
		digitalWrite(IO_PIN, !data);
		
		if (++io_bit >= 16) {
			 io_mode  = MODE_NONE;
		}

	/*
	} else {
		if (++io_bit > 7) {
			Serial.print("BAD I/O MODE\n");
			io_ltch   = 0;
			io_byte   = io_last;
		}*/
	}
}




////////////////////////////////////////////////////////////////////////////////
// CPU CORE 0 - HANDLES LED, NOTHING TO SETUP
////////////////////////////////////////////////////////////////////////////////
void setup() {
	Serial.begin(115200);
	delay(100);
}




////////////////////////////////////////////////////////////////////////////////
// SET UP... ALL THE THINGS !!!
// CPU CORE 1 - PROCESSES SNES/SFC ROM DATA
////////////////////////////////////////////////////////////////////////////////
void setup1() {
	// CONFIGURE OUR CLOCK AND DATA PINS
	pinMode(IO_PIN, OUTPUT);
	pinMode(CLOCK_PIN, INPUT_PULLUP);
	pinMode(LATCH_PIN, INPUT_PULLUP);

	// SET THE INITIAL BIT ON THE IO PIN
	digitalWrite(IO_PIN, 0);

	// CONFIGURE IRQ
	attachInterrupt(CLOCK_PIN, io_clock, FALLING);
	attachInterrupt(LATCH_PIN, io_latch, FALLING);
}




////////////////////////////////////////////////////////////////////////////////
// CPU CORE 0 - SHOW THE LED COLOR - THIS CHANGES WHEN TRANSFER BEGINS/ENDS
////////////////////////////////////////////////////////////////////////////////
void loop() {
	Serial.print(io_byte);
	Serial.print("\n");
	
	light.write(0, color_t::palette(io_byte & 0x0f));
	light.show();
	delay(200);
}




////////////////////////////////////////////////////////////////////////////////
// CPU CORE 1 - NOTHING TO DO IN LOOP, ROM DATA IS HANDLED BY IRQ
////////////////////////////////////////////////////////////////////////////////
void loop1() {}
