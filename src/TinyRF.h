#ifndef TINYRF_H
#define TINYRF_H

#include "Arduino.h"
#include "Settings.h"


//enable serial output if board is not ATtiny13 (assuming using MicroCore)
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


//data rate presets
#ifdef BITRATE_240
const unsigned int START_PULSE_PERIOD = 6000;
const unsigned int ONE_PULSE_PERIOD = 4000;
const unsigned int ZERO_PULSE_PERIOD = 3000;
const unsigned int PERIOD_HIGH_DURATION = 2000;
const unsigned int TRIGER_ERROR = 50;
const unsigned int START_PULSE_MAX_ERROR = 200;
const unsigned int NUM_PREAMBLE_BYTES = 3;
#endif

#ifdef BITRATE_500
const unsigned int START_PULSE_PERIOD = 3000;
const unsigned int ONE_PULSE_PERIOD = 2000;
const unsigned int ZERO_PULSE_PERIOD = 1500;
const unsigned int PERIOD_HIGH_DURATION = 1000;
const unsigned int TRIGER_ERROR = 50;
const unsigned int START_PULSE_MAX_ERROR = 200;
const unsigned int NUM_PREAMBLE_BYTES = 5;
#endif

#ifdef BITRATE_1100
const unsigned int START_PULSE_PERIOD = 2000;
const unsigned int ONE_PULSE_PERIOD = 1000;
const unsigned int ZERO_PULSE_PERIOD = 750;
const unsigned int PERIOD_HIGH_DURATION = 500;
const unsigned int TRIGER_ERROR = 30;
const unsigned int START_PULSE_MAX_ERROR = 100;
const unsigned int NUM_PREAMBLE_BYTES = 15;
#endif

#ifdef BITRATE_2500
const unsigned int START_PULSE_PERIOD = 2000;
const unsigned int ONE_PULSE_PERIOD = 400;
const unsigned int ZERO_PULSE_PERIOD = 300;
const unsigned int PERIOD_HIGH_DURATION = 200;
const unsigned int TRIGER_ERROR = 30;
const unsigned int START_PULSE_MAX_ERROR = 100;
const unsigned int NUM_PREAMBLE_BYTES = 30;
#endif


/**
 * Function declarations
**/
byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);


#endif  /* TINYRF_H */ 
