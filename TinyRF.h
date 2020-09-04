#ifndef TINYRF_H
#define TINYRF_H

#include "Arduino.h"

//Constants
const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;
//Do not increase this above 255, because all data types related to messages are uint8_t so 
//increasing this will break the program. 
//For example 'len' arugment of send() function is uint8_t so you can't 
//send anything longer than 255 bytes even if you increase this
const uint8_t MAX_MSG_LEN = 255;
//buffer size, i.e. number of bytes received that will be buffered
//when buffer is full it will start rewriting from the beggining
const uint16_t RX_BUFFER_SIZE = 256;
//preabmle to send before each transmission to get the receiver tuned
//increase this if you decrease pulse durations
//in my experience I needed ~30ms of preamble, Internet suggests much shorter times tho
const uint8_t NUM_PREAMBLE_BYTES = 4;
const uint8_t PREABMLE_DURATION = 30;


#define superfast

#ifdef slow
const unsigned int START_PULSE_DURATION = 8000;
const unsigned int ONE_PULSE_DURATION = 5000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif
 
#ifdef good
const unsigned int START_PULSE_DURATION = 6000;
const unsigned int ONE_PULSE_DURATION = 4000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif

#ifdef fast
const unsigned int START_PULSE_DURATION = 3000;
const unsigned int ONE_PULSE_DURATION = 2000;
const unsigned int ZERO_PULSE_DURATION = 1500;
const unsigned int HIGH_PERIOD_DURATION = 1000;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif

#ifdef superfast
const unsigned int START_PULSE_DURATION = 2000;
const unsigned int ONE_PULSE_DURATION = 1000;
const unsigned int ZERO_PULSE_DURATION = 800;
const unsigned int HIGH_PERIOD_DURATION = 500;
const int TRIGER_ERROR = 30;
const int START_PULSE_MAX_ERROR = 100;
#endif


//Function declarations
byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void interrupt_routine();
void send(byte data[], uint8_t len, uint8_t pin);
void transmitByte(byte _byte, uint8_t pin);
byte getReceivedData(byte buf[]);

#endif  /* TINYRF_H */ 
