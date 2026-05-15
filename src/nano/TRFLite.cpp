#ifndef TRFLITE_CPP_
#define TRFLITE_CPP_

#include "TRFLite.h"

uint8_t checksum8(uint16_t data)
{
	//compute the sum.  Let overflows accumulate in upper 8 bits
	uint16_t sum = 1;
	sum += (~(data << 8) & 0xFF);
	sum += (~data & 0xFF);

	//now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	return (byte) ~sum;
}

uint8_t crc8_fast(uint16_t data)
{
    // Start with a non-zero seed (0xAC is a common choice)
    // This ensures data = 0 doesn't result in CRC = 0
    uint8_t crc = 0xAC; 

    // Process high byte
    crc ^= (uint8_t)(data >> 8);
    for (uint8_t i = 0; i < 8; i++)
	{
        if (crc & 0x80) crc = (crc << 1) ^ 0x07; // Polynomial 0x07
        else crc <<= 1;
    }

    // Process low byte
    crc ^= (uint8_t)(data & 0xFF);
    for (uint8_t i = 0; i < 8; i++)
	{
        if (crc & 0x80) crc = (crc << 1) ^ 0x07;
        else crc <<= 1;
    }

    return crc;
}

#endif /* TRFLITE_CPP_ */