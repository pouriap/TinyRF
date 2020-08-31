#ifndef ATTTINY13_RF_H
#define ATTTINY13_RF_H

#include "Arduino.h"

const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;

byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void interrupt_routine();
void send(byte data[], uint8_t len, uint8_t pin);
void transmitByte(byte _byte, uint8_t pin);
byte getReceivedData(byte buf[]);

#endif  /* ATTTINY13_RF_H */ 
