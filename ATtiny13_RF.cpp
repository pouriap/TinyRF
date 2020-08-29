#include "Arduino.h"
#include "ATtiny13_RF.h"

/**
 * data: array of bytes
 * len: array size / number of bytes
**/
uint8_t checksum8(uint8_t data[], uint8_t len){
	uint16_t sum = 0;
	// Compute the sum.  Let overflows accumulate in upper 8 bits.
	for(uint8_t i=0; i<len; i++){
		//Serial.print("adding: ");Serial.print(~data[i]);Serial.print(" 0x");Serial.println(~data[i], HEX);
		sum += (~data[i] & 0xFF);
		//Serial.print("sum is: ");Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);
	}
	//Serial.print("sum after IF: "); Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);
	// Now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	//Serial.print("sum after carry: ");Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);

	// Return the 1s complement sum in finalsum
	return (uint8_t) ~sum;
}