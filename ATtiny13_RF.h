#ifndef ATTTINY13_RF_H
#define ATTTINY13_RF_H

#include "Arduino.h"

uint8_t checksum8(uint8_t data[], uint8_t len);
uint8_t crc8(uint8_t data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void send(char* data, uint8_t len, uint8_t pin);
void interrupt_routine();
void getReceivedData(uint8_t buf[]);
void transmitByte(char byte, uint8_t pin);

#endif  /* ATTTINY13_RF_H */ 
