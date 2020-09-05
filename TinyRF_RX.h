#ifndef TINYRF_RX_H
#define TINYRF_RX_H

#include "TinyRF.h"

//Constants
const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;

//buffer size, i.e. number of bytes received that will be buffered
//when buffer is full it will start rewriting from the beggining
const uint16_t RX_BUFFER_SIZE = 256;

#ifdef slow
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif
 
#ifdef good
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif

#ifdef fast
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400; 
#endif

#ifdef superfast
const int TRIGER_ERROR = 30;
const int START_PULSE_MAX_ERROR = 100;
#endif

#ifdef lightning
const int TRIGER_ERROR = 30;
const int START_PULSE_MAX_ERROR = 100;
#endif


void setupReceiver(uint8_t pin);
void interrupt_routine();
byte getReceivedData(byte buf[]);


#endif