#ifndef ATTTINY13_RF_H
#define ATTTINY13_RF_H

#include "Arduino.h"

uint8_t checksum8(uint8_t data[], uint8_t len);
void enableReceive(uint8_t portNumber);
void send(uint8_t data, uint8_t pin);
void interrupt_routine();
uint8_t getReceivedData();
unsigned long getDuration();

#endif  /* ATTTINY13_RF_H */
