#ifndef TRFLITE_TX_CPP_
#define TRFLITE_TX_CPP_

#include "TRFLite.h"
#include "TRFLite_TX.h"

void setupTransmitter()
{
    pinMode(TX_PIN, OUTPUT);
}

void send(uint16_t data)
{
    uint8_t crc = CRC_FUNC(data);

    //asm volatile("cli"); // Disable interrupts

    // preamble
    for(uint8_t i=0; i<10; i++)
    {
        // _sendBits(0xAAAA, 8);
        digitalWrite(TX_PIN, HIGH);
        _delay_us(DUR_ZERO + DUR_SPACE);
        digitalWrite(TX_PIN, LOW);
        _delay_us(DUR_ZERO + DUR_SPACE);
    }

    // send start
    digitalWrite(TX_PIN, HIGH);
    _delay_us(DUR_START);
    digitalWrite(TX_PIN, LOW);
    _delay_us(DUR_SPACE);

    // send crc
    _sendBits(crc, 8);

    // send data
    _sendBits(data, 16);

    //asm volatile("sei"); // Enable interrupts
}

void _sendBits(uint16_t data, uint8_t numBits)
{
    for(uint8_t txBitIndex=0; txBitIndex<numBits; txBitIndex++)
    {
        if (bitRead(data, txBitIndex))
        {
            digitalWrite(TX_PIN, HIGH);
            _delay_us(DUR_ONE);
            digitalWrite(TX_PIN, LOW);
            _delay_us(DUR_SPACE);
        }

        else
        {
            digitalWrite(TX_PIN, HIGH);
            _delay_us(DUR_ZERO);
            digitalWrite(TX_PIN, LOW);
            _delay_us(DUR_SPACE);
        }

    }
}

#endif /* TRFLITE_TX_CPP_ */