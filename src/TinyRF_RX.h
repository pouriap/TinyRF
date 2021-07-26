#ifndef TRF_RX_H
#define TRF_RX_H

#include "TinyRF.h"

/**
 * Board detection
**/
#if defined(__AVR_ATtiny2313__) || defined(__AVR_ATtiny4313__)
	#define TRF_MCU_TinyX313
#elif defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	#define TRF_MCU_TinyX4
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	#define TRF_MCU_TinyX5
#elif defined(__AVR_ATtiny167__) || defined(__AVR_ATtiny87__)
	#define TRF_MCU_TinyX7
#elif defined(ESP8266) || defined(ESP32)
	#define TRF_MCU_ESP
#endif


// fix for Digispark boards
//todo: find a way to make sure it's a digispark board
#if !defined(digitalPinToInterrupt)

	#ifndef NOT_AN_INTERRUPT
		#define NOT_AN_INTERRUPT -1
	#endif

	#if defined(TRF_MCU_TinyX313)
		#define digitalPinToInterrupt(p)  ((p) == 4 ? 0 : ((p) == 5 ? 1 : NOT_AN_INTERRUPT))

	#elif defined(TRF_MCU_TinyX4)
		#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : NOT_AN_INTERRUPT)

	#elif defined(TRF_MCU_TinyX5)
		#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : NOT_AN_INTERRUPT)

	#elif defined(TRF_MCU_TinyX7)
		#define digitalPinToInterrupt(p)  ((p) == 3 ? 0 : ((p) == 9 ? 1 : NOT_AN_INTERRUPT))

	#else
		#error "This board is not supported, please make an issue on the TinyRF github page to add this board."

	#endif

#endif


/**
 * Error codes returned by getReceivedData()
**/

#define TRF_ERR_SUCCESS 1
#define TRF_ERR_NO_DATA 2
#define TRF_ERR_CORRUPTED 3
#define TRF_ERR_BUFFER_OVERFLOW 4

//these are internal error codes that the user doesn't need to know about
#define TRF_ERR_DUPLICATE_MSG 10
//this is only used so that when we get noise in the buffer (all-zeroes or less than 3 byte frames)
//we won't return TRF_ERR_CORRUPTED, because there's no data to begin with
#define TRF_ERR_NOISE 11


/**
 * Function definitions
**/

// Sets up the receiver. Needs to be called in setup()
void setupReceiver(uint8_t pin);

void interrupt_routine();

// Gets "one" message from the received data buffer
// Number of received bytes will be put in numRcvdBytes
// Number of messages that were lost before this message will be put in numLostMsgs
// Returns one of the errors codes defined in TinyRF_RX.h
uint8_t getReceivedData(byte buf[], uint8_t bufSize, uint8_t &numRcvdBytes, uint8_t &numLostMsgs);

// Gets "one" message from the received data buffer
// Number of received bytes will be put in numRcvdBytes
// Returns one of the errors codes defined in TinyRF_RX.h
uint8_t getReceivedData(byte buf[],  uint8_t bufSize, uint8_t &numRcvdBytes);

// Functions for buffer addition/substraction
// They ensure a number doesn't surpass a certain value when added/substracted
void lim_inc(volatile uint8_t &num);
void lim_sum(volatile uint8_t &num, uint16_t c);
void lim_dec(volatile uint8_t &num);
void lim_sub(volatile uint8_t &num, uint16_t c);

#endif	/* TRF_RX_H */