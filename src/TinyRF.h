#ifndef TRF_H
#define TRF_H

#include "Arduino.h"
#include "Settings.h"

//#define DEBUG

//enable serial output if board is not ATtiny13
//used for developement debugging
#ifndef __AVR_ATtiny13__
	#define TRF_SERIAL_ENABLE
#endif


#if defined(TRF_SERIAL_ENABLE) && defined(DEBUG)
	#define TRF_SERIAL_BEGIN(x) (Serial.begin(x))
	#define TRF_PRINTLN(x) (Serial.println(x))
	#define TRF_PRINT(x) (Serial.print(x))
	#define TRF_PRINT2(x,y) (Serial.print(x,y))
#else
	#define TRF_SERIAL_BEGIN(x)
	#define TRF_PRINTLN(x)
	#define TRF_PRINT(x)
	#define TRF_PRINT2(x,y)
#endif


#if defined(TRF_TX_UNCALIBRATED)
	#define TRF_CALIB_ERROR 10
#elif defined(TRF_TX_CALIBRATED)
	#define TRF_CALIB_ERROR 2
#elif defined(TRF_TX_CRYSTAL)
	#define TRF_CALIB_ERROR 0
#endif


#if defined(TRF_ERROR_CHECKING_CRC)
	#define TRF_ERR_CHK_FUNC crc8
#elif defined(TRF_ERROR_CHECKING_CHECKSUM)
	#define TRF_ERR_CHK_FUNC checksum8
#endif


/**
 * This isn't used anywhere in the library. It is defined here for reference and for being used
 * as buffer size in programs that use this library.
 * Even tho sending messages of this length is theoretically possible, it is not recommended to 
 * send anything larger than 32 bytes. The longer a message is the more susceptible to noise it become
 * On top of that, the CRC function will become weaker and weaker as the message length increases
 * Also it will take more time to calculate the CRC, increasing transmission time and bugging down
 * the receiver CPU
**/
#define TRF_MAX_MSG_LEN TRF_RX_BUFFER_SIZE-5	//-5 to account for len, CRC, seq# and just to be safe



//data rate presets
//these numbers are magic numbers, do not change them
//they use the power of magic to reduce memory usage
//if you change them even slightly you will see that memory usage will increase
#ifdef TRF_BITRATE_200
	const uint16_t START_PULSE_PERIOD = 5996;
	const uint16_t ONE_PULSE_PERIOD = 3996;
	const uint16_t ZERO_PULSE_PERIOD = 3001;
	const uint16_t PERIOD_HIGH_DURATION = 1998;
	const uint16_t TX_INTERVAL_CONST = 2999;
	const uint16_t TRIGGER_ERROR = 50;
	const uint16_t NUM_PREAMBLE_BYTES = 3;
#endif

#ifdef TRF_BITRATE_500
	const uint16_t START_PULSE_PERIOD = 3001;
	const uint16_t ONE_PULSE_PERIOD = 2001;
	const uint16_t ZERO_PULSE_PERIOD = 1501;
	const uint16_t PERIOD_HIGH_DURATION = 1003;
	const uint16_t TX_INTERVAL_CONST = 5997;
	const uint16_t TRIGGER_ERROR = 50;
	const uint16_t NUM_PREAMBLE_BYTES = 6;
#endif

#ifdef TRF_BITRATE_1000
	const uint16_t START_PULSE_PERIOD = 1998;
	const uint16_t ONE_PULSE_PERIOD = 1001;
	const uint16_t ZERO_PULSE_PERIOD = 751;
	const uint16_t PERIOD_HIGH_DURATION = 498;
	const uint16_t TX_INTERVAL_CONST = 10002;
	const uint16_t TRIGGER_ERROR = 30;
	const uint16_t NUM_PREAMBLE_BYTES = 15;
#endif

#ifdef TRF_BITRATE_2000
	#ifdef TRF_TX_UNCALIBRATED
		#warning "This data rate is too fast for an uncalibrated ATtiny."
	#endif
	const uint16_t START_PULSE_PERIOD = 1998;
	const uint16_t ONE_PULSE_PERIOD = 601;
	const uint16_t ZERO_PULSE_PERIOD = 451;
	const uint16_t PERIOD_HIGH_DURATION = 298;
	const uint16_t TX_INTERVAL_CONST = 8002;
	const uint16_t TRIGGER_ERROR = 30;
	const uint16_t NUM_PREAMBLE_BYTES = 30;
#endif

const uint16_t ONE_PULSE_TRIGG_ERROR = (TRIGGER_ERROR + ONE_PULSE_PERIOD * TRF_CALIB_ERROR / 100);
const uint16_t ZERO_PULSE_TRIGG_ERROR = (TRIGGER_ERROR + ZERO_PULSE_PERIOD * TRF_CALIB_ERROR / 100);
const uint16_t START_PULSE_TRIGG_ERROR = (TRIGGER_ERROR + START_PULSE_PERIOD * TRF_CALIB_ERROR / 100);
//for some reason longer delays are more inaccurate, so our start pulse maximum needs more leeway for error
const uint16_t START_PULSE_MAX_ERROR = 2*START_PULSE_TRIGG_ERROR;

const uint16_t MIN_TX_INTERVAL_REAL = START_PULSE_PERIOD + START_PULSE_MAX_ERROR;
//this is for end-user usage 
const uint16_t TX_DELAY_MICROS = MIN_TX_INTERVAL_REAL * 2;


/**
 * Function declarations
**/
byte checksum8(byte data[], uint8_t len, uint8_t seq = 0);
byte crc8(byte data[], uint8_t len, uint8_t seq = 0);


#endif  /* TRF_H */ 
