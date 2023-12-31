#ifndef __led_h__
#define __led_h__




#include "../color/color.h"
#include "interrupt.h"




enum LED_MODE {
	LED_RGB		= 0x01,
	LED_GRB		= 0x02,
	LED_WRGB	= 0x09,
	LED_WGRB	= 0x0A,
};


enum LED_TYPE {
	LED_BASIC	= 0x10,
	LED_ARRAY	= 0x20,
	LED_GRID	= 0x30,
};




class led {
	public:




	////////////////////////////////////////////////////////////////////////////
	// CONSTRUCTOR - PASS IN THE PIN FOR THE LEDS, AS WELL AS THE LED COUNT
	////////////////////////////////////////////////////////////////////////////
	INLINE led(uint8_t led_pin, uint16_t led_total, LED_MODE led_mode=LED_GRB) {
		_pin	= led_pin;
		_total	= led_total;
		_mode	= led_mode;
		_type	= LED_BASIC;

		clock();
	}




	////////////////////////////////////////////////////////////////////////////
	// RE-CALCULATE THE CURRENT CLOCK SPEED AND DELAYS
	////////////////////////////////////////////////////////////////////////////
	void clock();




	////////////////////////////////////////////////////////////////////////////
	// DRAW A SINGLE PIXEL TO THE LED STRIP - USING COLOR_T STRUCT
	// NOTE: implemented differently for each board.
	// This is the only unique method between implementations.
	// See existing versions for an idea of how to port this to a new board.
	////////////////////////////////////////////////////////////////////////////
	void pixel(const color_t &color);




	////////////////////////////////////////////////////////////////////////////
	// DRAW A SINGLE PIXEL, SIMILAR TO ABOVE
	////////////////////////////////////////////////////////////////////////////
	INLINE void pixel(volatile const color_t &color) {
		pixel(color_t(color));
	}




	////////////////////////////////////////////////////////////////////////////
	// DRAW A SINGLE PIXEL TO THE LED STRIP - USING INDIVIDUAL R,G,B VALUES
	////////////////////////////////////////////////////////////////////////////
	INLINE void pixel(uint8_t r, uint8_t g, uint8_t b) {
		pixel(color_t(r, g, b));
	}




	////////////////////////////////////////////////////////////////////////////
	// CLEAR THE ENTIRE PIXEL ARRAY
	////////////////////////////////////////////////////////////////////////////
	void clear() {
		begin();

		for (uint16_t i=0; i<_total; i++) {
			pixel(0, 0, 0);
		}

		end();
	}




	////////////////////////////////////////////////////////////////////////////
	// INITIAL SETUP OF THE LED STRIP - CALL AT BEGINNING OF RENDER LOOP
	////////////////////////////////////////////////////////////////////////////
	INLINE void begin() {
		intr_disable();
		pinMode(_pin, OUTPUT);
	}




	////////////////////////////////////////////////////////////////////////////
	// FINALIZE AND SHOW LED STRIP - CALL AT END OF RENDER LOOP
	////////////////////////////////////////////////////////////////////////////
	INLINE void end() {
		digitalWrite(_pin, LOW);
		intr_enable();
	}




	////////////////////////////////////////////////////////////////////////////
	// GET THE PIN USED FOR THIS LED STRIP
	////////////////////////////////////////////////////////////////////////////
	INLINE uint8_t pin() const {
		return _pin;
	}




	////////////////////////////////////////////////////////////////////////////
	// GET THE NUMBER OF LEDS IN THIS STRIP
	////////////////////////////////////////////////////////////////////////////
	INLINE uint16_t total() const {
		return _total;
	}




	////////////////////////////////////////////////////////////////////////////
	// GET THE CURRENT LED STRIP MODE: RGB VS GRB DATA ORDER
	////////////////////////////////////////////////////////////////////////////
	INLINE LED_MODE mode() const {
		return _mode;
	}




	////////////////////////////////////////////////////////////////////////////
	// SET THE CURRENT LED STRIP MODE: RGB VS GRB DATA ORDER
	////////////////////////////////////////////////////////////////////////////
	INLINE LED_MODE mode(LED_MODE led_mode) {
		LED_MODE mode = _mode;
		_mode = led_mode;
		return mode;
	}




	////////////////////////////////////////////////////////////////////////////
	// GET THE CURRENT LED RENDERING TYPE
	////////////////////////////////////////////////////////////////////////////
	INLINE LED_TYPE type() {
		return _type;
	}




	private:
		uint8_t		_pin;
		LED_MODE	_mode;
		uint16_t	_total;


#if defined(ARDUINO_ARCH_RP2040) || defined(ESP8266) || defined(ESP32)
		uint32_t	CYCLES_T0H;
		uint32_t	CYCLES_T1H;
		uint32_t	CYCLES;
#endif // defined(ARDUINO_ARCH_RP2040) || defined(ESP8266) || defined(ESP32)


	protected:
		LED_TYPE	_type;
};




#endif //__led_h__
