#include "TinyRF.h"

//todo: try different CPU speeds
//todo: receiver is very sensitive to preamble, and in higher data rates we need more preable
//todo: sometimes getreceivedata() gives blank string, specially when we are sending fast (1ms delay between sends)
/* whit this code in receiver:
void loop(){
  char buf[TRF_MAX_MSG_LEN];
  for(int i=0; i<TRF_MAX_MSG_LEN; i++){
	  buf[i] = '\0';
  }
  uint8_t err = getReceivedData(buf);
  if(err == TRF_ERR_SUCCESS){
	  TRF_PRINTLN(buf);
  }
  if(err != TRF_ERR_NO_DATA){
  	TRF_PRINTLN("---------------------------------");
  }
}

*/

/*
notes:
- if we had errors when increasing data rates it's because of noise and we should enable MAX values for
pulse periods to increase accuracy
- if we had errors when increasing gap between messages it's because of preable 
*/


//1-byte 1-s compliment checksum, supposedly the algorithm used in TCP
byte checksum8_seq(byte data[], uint8_t len, uint8_t seq){
	uint16_t sum = 0;
	// Compute the sum.  Let overflows accumulate in upper 8 bits.
	sum += (~seq & 0xFF);
	for(uint8_t i=0; i<len; i++){
		sum += (~data[i] & 0xFF);
	}
	// Now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	return (byte) ~sum;
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte crc8_seq(byte data[], uint8_t len, uint8_t seq){
	byte crc = 0x00;
	byte extract = seq;
	do
	{
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
		extract = *data++;
	}while (len--);
	return crc;
}

//1-byte 1-s compliment checksum, supposedly the algorithm used in TCP
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
