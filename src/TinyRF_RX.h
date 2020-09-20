#ifndef TRF_RX_H
#define TRF_RX_H

#include "TinyRF.h"


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