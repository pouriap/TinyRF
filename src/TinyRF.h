#ifndef TINYRF_H
#define TINYRF_H

#include "Arduino.h"

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

/**
 * This isn't used anywhere in the library. It is defined here for reference and for being used
 * as buffer size in programs that use this library.
 * Do not increase this above 255, because all data types related to messages are uint8_t so 
 * increasing this will break the program.
 * Even tho sending messages of this length is theoretically possible, it is not recommended to 
 * send anything larger thatn 128 bit due to noise.
**/
const uint8_t MAX_MSG_LEN = 255;


/**
 * We don't have a specific pulse that signals the end of a transmission. We rely on noise for that.
 * When noise is received in the receiver the transmission is considered over.
 * But in the rare even that you are in a noiseless environment this could mean that the receiver 
 * will keep waiting for the next byte of the transmission. There are two ways we can fix that
 * 1- Create noise in the TX: i.e. send a bunch of meaningless pulses 
 * 2- Detect end of transmission in RX: i.e. when no data has been received for a while considere the transmission finished
 * The default is EOT_IN_RX because we want to minimize the transmitter code size
 * You can uncomment EOT_IN_TX if you want EOT to be done in transmitter
 * Alternatively you can uncomment EOT_NONE if you think you don't need this
**/
//#define EOT_IN_TX
//#define EOT_NONE

/**
 * What kind of error checking should be used
 * CRC detects more errors but uses ~20bytes more program space
 * Checksum detects less errors
 * Alternatively you can use ERROR_CHECKING_NONE to disable error checking
 * Note that error checking can only detect if a data was curropted during transmission. 
 * It cannot recover the original data. So you still need to send a message multiple times to 
 * make sure it is received.
**/
#define ERROR_CHECKING crc8
//#define ERROR_CHECKING checksum8
//#define ERROR_CHECKING_NONE

/**
 * The pin that is connected to the transmission module
 * We use a #define instead of settting it programatically in order to save program space on TX
**/
#define txPin 2


/**
 * Data rate presets
 * According to the datasheet uncalibrated ATtiny13 has 10% frequency error.
 * We also need at least 50us of error margin because we use delayMicroseconds() which isn't super accurate.
 * In my experience "fast" was the best I could do with ATtiny13 as TX
 * If you are using an Arduino as TX you can go up to "lightning" speed
 *
 * Preabmle:
 * Preabmle to send before each transmission to get the receiver tuned.
 * Increase preabmle if you decrease pulse periods.
 * In my experiments I needed ~50ms of preamble (the faster the datarate the more preabmle needed).
 * However Internet suggests much shorter times.
**/
//todo: it is best to not use bytes as preabmle for very fast datarates, instead send raw HIGH,LOW
#define superfast

#ifdef slow
const unsigned int START_PULSE_PERIOD = 8000;
const unsigned int ONE_PULSE_PERIOD = 5000;
const unsigned int ZERO_PULSE_PERIOD = 3000;
const unsigned int PERIOD_HIGH_DURATION = 2000;
const unsigned int TRIGER_ERROR = 50;
const unsigned int START_PULSE_MAX_ERROR = 200;
const unsigned int NUM_PREAMBLE_BYTES = 3;
#endif
 
#ifdef good
const unsigned int START_PULSE_PERIOD = 6000;
const unsigned int ONE_PULSE_PERIOD = 4000;
const unsigned int ZERO_PULSE_PERIOD = 3000;
const unsigned int PERIOD_HIGH_DURATION = 2000;
const unsigned int TRIGER_ERROR = 50;
const unsigned int START_PULSE_MAX_ERROR = 200;
const unsigned int NUM_PREAMBLE_BYTES = 3;
#endif

#ifdef fast
const unsigned int START_PULSE_PERIOD = 3000;
const unsigned int ONE_PULSE_PERIOD = 2000;
const unsigned int ZERO_PULSE_PERIOD = 1500;
const unsigned int PERIOD_HIGH_DURATION = 1000;
const unsigned int TRIGER_ERROR = 50;
const unsigned int START_PULSE_MAX_ERROR = 200;
const unsigned int NUM_PREAMBLE_BYTES = 5;
#endif

#ifdef superfast
const unsigned int START_PULSE_PERIOD = 2000;
const unsigned int ONE_PULSE_PERIOD = 1000;
const unsigned int ZERO_PULSE_PERIOD = 750;
const unsigned int PERIOD_HIGH_DURATION = 500;
const unsigned int TRIGER_ERROR = 30;
const unsigned int START_PULSE_MAX_ERROR = 100;
const unsigned int NUM_PREAMBLE_BYTES = 15;
#endif

#ifdef lightning
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
