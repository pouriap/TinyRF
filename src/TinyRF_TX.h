#ifndef TRF_TX_H
#define TRF_TX_H

#include "TinyRF.h"

//Function declarations
void setupTransmitter();
void send(byte data[], uint8_t len);
void send(byte data[], uint8_t len, uint8_t times);
void transmitByte(byte _byte);


#endif	/* TRF_TX_H */