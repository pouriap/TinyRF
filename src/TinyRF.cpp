#include "TinyRF.h"

//1-byte 1-s compliment checksum, supposedly the algorithm used in TCP
byte checksum8(byte data[], uint8_t len, uint8_t seq){
	//compute the sum.  Let overflows accumulate in upper 8 bits
	uint16_t sum = 0;
	sum += (~seq & 0xFF);
	for(uint8_t i=0; i<len; i++){
		sum += (~data[i] & 0xFF);
	}
	//now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	return (byte) ~sum;
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte crc8(byte data[], uint8_t len, uint8_t seq){
	byte crc = seq;
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
