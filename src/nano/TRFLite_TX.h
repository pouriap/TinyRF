#ifndef TRFLITE_TX_H_
#define TRFLITE_TX_H_

// if we let the user choose TX pin in setupTransmitter it will add 130 bytes of overhead so I decided not to
// it can still be changed from here to any pin
#define TX_PIN 0

void setupTransmitter();
void send(uint16_t data);
void _sendBits(uint16_t data, uint8_t numBits);

#endif    // TRFLITE_TX_H_
