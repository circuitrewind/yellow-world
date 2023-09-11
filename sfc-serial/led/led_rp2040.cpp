////////////////////////////////////////////////////////////////////////////////
// https://cec-code-lab.aps.edu/robotics/resources/pico-c-api/systick_8h_source.html
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////////////////////////
#include <arduino.h>
#include "led.h"




////////////////////////////////////////////////////////////////////////////////
// THIS FILE ONLY APPLIES TO THE RASPBERRY PI RO2040 BASED CHIPS/BOARDS
////////////////////////////////////////////////////////////////////////////////
#if defined(ARDUINO_ARCH_RP2040)




////////////////////////////////////////////////////////////////////////////////
// DYNAMICALLY CALCULATE THE CLOCK DELAYS
////////////////////////////////////////////////////////////////////////////////
void led::clock() {
	float clockspeed;

	clockspeed	= static_cast<float>(rp2040.f_cpu());
	CYCLES_T0H	= static_cast<uint32_t>(clockspeed * 0.000000350f);
	CYCLES_T1H	= static_cast<uint32_t>(clockspeed * 0.000000700f);
	CYCLES		= static_cast<uint32_t>(clockspeed * 0.000001250f);
}




////////////////////////////////////////////////////////////////////////////////
// SEND A FULL PIXEL TO THE LED STRIP
////////////////////////////////////////////////////////////////////////////////
void led::pixel(const color_t &color) {
	uint32_t	mask	= 1 << _pin;
	uint32_t	data	= (_mode==LED_RGB) ? ((uint32_t)color) : (color.grb());
	uint32_t	start	= 0;
	uint32_t	pause	= 0;

	for (int32_t i=23; i>=0; i--) {
		pause = (data & (1<<i)) ? CYCLES_T1H : CYCLES_T0H;
		start = systick_hw->cvr;

		//WRITE HIGH VALUE AND PAUSE
		sio_hw->gpio_set = mask;
		while ((start - systick_hw->cvr) < pause) {}

		//WRITE LOW VALUE AND PAUSE
		sio_hw->gpio_clr = mask;
		while ((start - systick_hw->cvr) < CYCLES) {}
	}
}




#endif //defined(ARDUINO_ARCH_RP2040)
