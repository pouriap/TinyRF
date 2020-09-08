#ifndef TINYRF_TX_H
#define TINYRF_TX_H

#include "TinyRF.h"

#ifndef TX_NO_SEQ
	//add 1 to the buffer length to store the seq# in
	#define TINYRF_DATA_LEN(bufLen) (bufLen+1)
#else
	#define TINYRF_DATA_LEN(bufLen) (bufLen)
#endif

//Function declarations
void setupTransmitter();
void send(byte data[], uint8_t len);
void transmitByte(byte _byte);


#endif	/* TINYRF_TX_H */