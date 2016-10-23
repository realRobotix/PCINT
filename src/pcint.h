/********************
Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
adapted from: http://playground.arduino.cc/Main/PcInt
with many changes to make it compatible with arduino boards (original worked on Uno)
not tested on many boards but has the code uses arduino macros for pin mappings
it should be more compatible and also more easy to extend
some boards migh need pcintPinMap definition

Sept. 2014
  small changes to existing PCINT library, supporting an optional cargo parameter

Nov.2014 large changes
  - Use arduino standard macros for PCINT mapping instead of specific map math, broaden compatibility
  - array[a][b] is 17% faster than array[(a<<3)+b], same memory
  - reverse pin mappings for pin change check (not on arduino env. AFAIK)

**/
#ifndef ARDUINO_PCINT_MANAGER
#define ARDUINO_PCINT_MANAGER
	#if ARDUINO < 100
	#include <WProgram.h>
	#else
	#include <Arduino.h>
	#endif
	#include "pins_arduino.h"

	// PCINT reverse map
	// because some avr's (like 2560) have a messed map we got to have this detailed pin reverse map
	// still this makes the PCINT automatization very slow, risking interrupt collision
	#if defined(digital_pin_to_pcint)
		#define digitalPinFromPCINTSlot(slot,bit) pgm_read_byte(digital_pin_to_pcint+(((slot)<<3)+(bit)))
		#define pcintPinMapBank(slot) ((uint8_t*)((uint8_t*)digital_pin_to_pcint+((slot)<<3)))
	#else
		#if ( defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega16u4__) )
			//UNO
			//const uint8_t PROGMEM pcintPinMap[3][8]={{8,9,10,11,12,13,-1,-1},{A0,A1,A2,A3,A4,A5,-1,-1},{0,1,2,3,4,5,6,7}};
			const uint8_t PROGMEM pcintPinMap[3][8]={{8,9,10,11,12,13,-1,-1},{14,15,16,17,18,19,20,21},{0,1,2,3,4,5,6,7}};
		#elif ( defined(__AVR_ATmega2560__) )
			const uint8_t PROGMEM pcintPinMap[3][8]={{53,52,51,50,10,11,12,13},{0,15,14,-1,-1,-1,-1,-1},{A8,A9,A10,A11,A12,A13,A14,A15}};
		#elif ( defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega644__))
			//PCINT map for Bobuino and Skinny Bob
			//const uint8_t PROGMEM pcintPinMap[3][8]={{21,20,19,18,17,16,15,14},{4,5,6,7,10,11,12,13},{22,23,24,25,26,27,28,29}};
			//PCINT map for Arduinpo Uno*Pro
			//const uint8_t PROGMEM pcintPinMap[3][8]={{21,20,19,18,17,16,15,14},{4,5,2,3,10,11,12,13},{22,23,24,25,26,27,28,29}};
			#error "uC PCINT REVERSE MAP IS NOT DEFINED, ATmega1284P variant unknown"
		#else
			#error "uC PCINT REVERSE MAP IS NOT DEFINED"
		#endif
		#define digitalPinFromPCINTSlot(slot,bit) pgm_read_byte(pcintPinMap+(((slot)<<3)+(bit)))
		#define pcintPinMapBank(slot) ((uint8_t*)((uint8_t*)pcintPinMap+((slot)<<3)))
	#endif
	#define digitalPinFromPCINTBank(bank,bit) pgm_read_byte((uint8_t*)bank+bit)

	//this handler can be used instead of any void(*)() and optionally it can have an associated void *
	//and use it to call void(*)(void* payload)
	struct mixHandler {
		union {
			void (*voidFunc)(void);
			void (*payloadFunc)(void*);
		} handler;
		void *payload;
		inline mixHandler():payload(NULL) {handler.voidFunc=NULL;}
		inline mixHandler(void (*f)(void)):payload(NULL) {handler.voidFunc=f;}
		inline mixHandler(void (*f)(void*),void *payload):payload(payload) {handler.payloadFunc=f;}
		inline void operator()() {payload?handler.payloadFunc(payload):handler.voidFunc();}
		inline bool operator==(void*ptr) {return handler.voidFunc==ptr;}
		inline bool operator!=(void*ptr) {return handler.voidFunc!=ptr;}
	};

	typedef void (*voidFuncPtr)(void);

	#define HANDLER_TYPE mixHandler

	/*
	 * attach an interrupt to a specific pin using pin change interrupts.
	 */
	void PCattachInterrupt(uint8_t pin, class mixHandler userFunc, uint8_t mode);

	void PCdetachInterrupt(uint8_t pin);

	// common code for isr handler. "port" is the PCINT number.
	// there isn't really a good way to back-map ports and masks to pins.
	// here we consider only the first change found ignoring subsequent, assuming no interrupt cascade
	static void PCint(uint8_t port);

#endif
