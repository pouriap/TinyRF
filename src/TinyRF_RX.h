#ifndef TINYRF_RX_H
#define TINYRF_RX_H

#include "TinyRF.h"


//Constants
const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;


/**
 * Buffer size, i.e. number of bytes received that will be buffered.
 * When buffer is full it will start rewriting from the beggining
**/
const uint16_t RX_BUFFER_SIZE = 256;


/**
 * Function definitions
**/
void setupReceiver(uint8_t pin);
void interrupt_routine();
byte getReceivedData(byte buf[]);


#endif	/* TINYRF_RX_H */