#ifndef ATTTINY13_RF_H
#define ATTTINY13_RF_H

#include "Arduino.h"

//Constants
const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;
//Do not increase this, because all data types related to messages are uint8_t so increasing this
//will break the program. For example 'len' arugment of send() function is uint8_t so you can't 
//send anything longer thatn 255 bytes even if you increase this
const uint8_t MAX_MSG_LEN = 255;
//preabmle to send before each transmission to get the receiver tuned
//increase this if you decrease pulse durations
//in my experience I needed ~30ms of preamble, Internet suggests much shorter times tho
const uint8_t NUM_PREAMBLE_BYTES = 4;
/*

//super long
const unsigned int START_PULSE_DURATION = 8000;
const unsigned int ONE_PULSE_DURATION = 5000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
const int TRIGER_ERROR = 50;
*/


//good
const unsigned int START_PULSE_DURATION = 6000;
const unsigned int ONE_PULSE_DURATION = 4000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 500;


/*
//fast
const unsigned int START_PULSE_DURATION = 3000;
const unsigned int ONE_PULSE_DURATION = 2000;
const unsigned int ZERO_PULSE_DURATION = 1500;
const unsigned int HIGH_PERIOD_DURATION = 1000;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
*/


//super fast
/*
const unsigned int START_PULSE_DURATION = 2000;
const unsigned int ONE_PULSE_DURATION = 1000;
const unsigned int ZERO_PULSE_DURATION = 800;
const unsigned int HIGH_PERIOD_DURATION = 500;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
*/


//Function declarations
byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void interrupt_routine();
void send(byte data[], uint8_t len, uint8_t pin);
void transmitByte(byte _byte, uint8_t pin);
byte getReceivedData(byte buf[]);

#endif  /* ATTTINY13_RF_H */ 
