#ifndef ATTTINY13_RF_H
#define ATTTINY13_RF_H

#include "Arduino.h"

const uint8_t TINYRF_ERR_NO_DATA = 0;
const uint8_t TINYRF_ERR_BAD_CRC = 1;
const uint8_t TINYRF_ERR_SUCCESS = 2;

uint8_t checksum8(uint8_t data[], uint8_t len);
uint8_t crc8(uint8_t data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void send(char* data, uint8_t len, uint8_t pin);
void interrupt_routine();
uint8_t getReceivedData(uint8_t buf[]);
void transmitByte(char byte, uint8_t pin);

#endif  /* ATTTINY13_RF_H */ 
