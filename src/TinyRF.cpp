#include "TinyRF.h"

//todo: receiver is very sensitive to preamble, and in higher data rates we need more preable
//todo: change pulse duration to pulse period

/*
notes:
- if we had errors when increasing data rates it's because of noise and we should enable MAX values for
pulse durations to increase accuracy
- if we had errors when increasing gap between messages it's because of preable 
*/


//one bytes 1-s compliment checksum, supposedly the algorithm used in TCP
byte checksum8(byte data[], uint8_t len){
	uint16_t sum = 0;
	// Compute the sum.  Let overflows accumulate in upper 8 bits.
	for(uint8_t i=0; i<len; i++){
		sum += (~data[i] & 0xFF);
	}
	// Now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	return (byte) ~sum;
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte crc8(byte data[], uint8_t len){
	byte crc = 0x00;
	while (len--)
	{
		byte extract = *data++;
		for (byte tempI = 8; tempI; tempI--)
		{
			byte sum = (crc ^ extract) & 0x01;
			crc >>= 1;
			if (sum)
			{
				crc ^= 0x8C;
			}
			extract >>= 1;
		}
	}
	return crc;
}
