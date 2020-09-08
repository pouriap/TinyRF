#ifndef TINYRF_H
#define TINYRF_H

#include "Arduino.h"
#include "Settings.h"


//enable serial output if board is not ATtiny13 (assuming using MicroCore)
//used for developement debugging
#ifndef __AVR_ATtiny13__
	#define SERIAL_ENABLE
#endif

#ifdef SERIAL_ENABLE
	#define TINYRF_SERIAL_BEGIN(x) (Serial.begin(x))
	#define TINYRF_PRINTLN(x) (Serial.println(x))
	#define TINYRF_PRINT(x) (Serial.print(x))
	#define TINYRF_PRINT2(x,y) (Serial.print(x,y))
#else
	#define TINYRF_SERIAL_BEGIN(x)
	#define TINYRF_PRINTLN(x)
	#define TINYRF_PRINT(x)
	#define TINYRF_PRINT2(x,y)
#endif


#ifdef TX_ATTINY_UNCALIBRATED
	#define CALIB_ERROR 10
#else
	#define CALIB_ERROR 0
#endif

#ifndef TX_NO_SEQ
	#if defined(ERROR_CHECKING_CRC)
		#define ERR_CHK_FUNC crc8_seq
	#elif defined(ERROR_CHECKING_CHECKSUM)
		#define ERR_CHK_FUNC checksum8_seq
	#endif
#else
	#if defined(ERROR_CHECKING_CRC)
		#define ERR_CHK_FUNC crc8
	#elif defined(ERROR_CHECKING_CHECKSUM)
		#define ERR_CHK_FUNC checksum8
	#endif
#endif


//data rate presets
#ifdef BITRATE_240
	const uint16_t START_PULSE_PERIOD = 6000;
	const uint16_t ONE_PULSE_PERIOD = 4000;
	const uint16_t ZERO_PULSE_PERIOD = 3000;
	const uint16_t PERIOD_HIGH_DURATION = 2000;
	const uint16_t TRIGER_ERROR = 50;
	const uint16_t START_PULSE_MAX_ERROR = 200;
	const uint16_t NUM_PREAMBLE_BYTES = 3;
#endif

#ifdef BITRATE_500
	const uint16_t START_PULSE_PERIOD = 3000;
	const uint16_t ONE_PULSE_PERIOD = 2000;
	const uint16_t ZERO_PULSE_PERIOD = 1500;
	const uint16_t PERIOD_HIGH_DURATION = 1000;
	const uint16_t TRIGER_ERROR = 50;
	const uint16_t START_PULSE_MAX_ERROR = 200;
	const uint16_t NUM_PREAMBLE_BYTES = 5;
#endif

#ifdef BITRATE_1100
	const uint16_t START_PULSE_PERIOD = 2000;
	const uint16_t ONE_PULSE_PERIOD = 1000;
	const uint16_t ZERO_PULSE_PERIOD = 750;
	const uint16_t PERIOD_HIGH_DURATION = 500;
	const uint16_t TRIGER_ERROR = 30;
	const uint16_t START_PULSE_MAX_ERROR = 100;
	const uint16_t NUM_PREAMBLE_BYTES = 15;
#endif

#ifdef BITRATE_2500
	#ifdef TX_ATTINY_UNCALIBRATED
		#warning "This data rate is too fast for an uncalibrated ATtiny."
	#endif
	const uint16_t START_PULSE_PERIOD = 2000;
	const uint16_t ONE_PULSE_PERIOD = 400;
	const uint16_t ZERO_PULSE_PERIOD = 300;
	const uint16_t PERIOD_HIGH_DURATION = 200;
	const uint16_t TRIGER_ERROR = 30;
	const uint16_t START_PULSE_MAX_ERROR = 100;
	const uint16_t NUM_PREAMBLE_BYTES = 30;
#endif

const uint16_t ONE_PULSE_TRIGG_ERROR = (TRIGER_ERROR + ONE_PULSE_PERIOD * CALIB_ERROR / 100);
const uint16_t ZERO_PULSE_TRIGG_ERROR = (TRIGER_ERROR + ZERO_PULSE_PERIOD * CALIB_ERROR / 100);
const uint16_t START_PULSE_TRIGG_ERROR = (TRIGER_ERROR + START_PULSE_PERIOD * CALIB_ERROR / 100);


/**
 * Function declarations
**/
byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);
byte checksum8_seq(byte data[], uint8_t len, uint8_t seq);
byte crc8_seq(byte data[], uint8_t len, uint8_t seq);


#endif  /* TINYRF_H */ 
