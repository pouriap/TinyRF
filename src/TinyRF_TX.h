#ifndef TRF_TX_H
#define TRF_TX_H

#include "TinyRF.h"


/**
 * Function declarations
**/

void setupTransmitter();

// Sends an array of bytes
// Also appends error checking and sequence numbering
void send(byte data[], uint8_t len);

// Sends an array of bytes 'times' times
void send(byte data[], uint8_t len, uint8_t times);

// Sends one byte to the receiver
void transmitByte(byte _byte);


#endif	/* TRF_TX_H */