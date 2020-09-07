#ifndef TINYRF_TX_H
#define TINYRF_TX_H

#include "TinyRF.h"

//preabmle to send before each transmission to get the receiver tuned
//increase this if you decrease pulse durations
//in my experiments I needed ~50ms of preamble (the faster the datarate the more preabmle needed)
//however Internet suggests much shorter times, radiohead for example uses 36bits at 2000 baud rate = ~18ms
const uint8_t PREABMLE_DURATION = 50;

//Function declarations
void setupTransmitter();
void send(byte data[], uint8_t len);
void transmitByte(byte _byte);


#endif