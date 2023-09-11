////////////////////////////////////////////////////////////////////////////////
//INCLUDE FILES
////////////////////////////////////////////////////////////////////////////////
#include <arduino.h>
#include "led.h"




////////////////////////////////////////////////////////////////////////////////
//THIS FILE ONLY APPLIES TO THE ESP8266, ESP32, AND NODEMCU CHIPS/BOARDS
////////////////////////////////////////////////////////////////////////////////
#if defined(ESP8266) || defined(ESP32)




////////////////////////////////////////////////////////////////////////////////
// GET THE NUMBER OF CPU CYCLES SINCE POWER ON
////////////////////////////////////////////////////////////////////////////////
static INLINE uint32_t esp_cycle() {
	uint32_t ccount;
	__asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
	return ccount;
}




////////////////////////////////////////////////////////////////////////////////
// DYNAMICALLY CALCULATE THE CLOCK DELAYS
////////////////////////////////////////////////////////////////////////////////
void led::clock() {
	float clockspeed;

	clockspeed	= static_cast<float>(getCpuFrequencyMhz());
	CYCLES_T0H	= static_cast<uint32_t>(clockspeed * 0.000000350f);
	CYCLES_T1H	= static_cast<uint32_t>(clockspeed * 0.000000700f);
	CYCLES		= static_cast<uint32_t>(clockspeed * 0.000001250f);
}




////////////////////////////////////////////////////////////////////////////////
// SEND A FULL PIXEL TO THE LED STRIP
////////////////////////////////////////////////////////////////////////////////
void ICACHE_RAM_ATTR led::pixel(const color_t &color) {
	uint32_t	value	= 1 << _pin;
	uint32_t	data	= (_mode==LED_RGB) ? ((uint32_t)color) : (color.grb());
	uint32_t	start	= 0;
	uint32_t	pause	= 0;

	for (int32_t i=23; i>=0; i--) {
		pause = (data & (1<<i)) ? CYCLES_T1H : CYCLES_T0H;
		start = esp_cycle();

		//WRITE HIGH VALUE AND PAUSE
		GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, value);
		while ((esp_cycle() - start) < pause) {}

		//WRITE LOW VALUE AND PAUSE
		GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, value);
		while ((esp_cycle() - start) < CYCLES) {}
	}
}




#endif //defined(ESP8266) || defined(ESP32)
