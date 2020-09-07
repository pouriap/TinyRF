#include "TinyRF_TX.h"

void setupTransmitter(){
	pinMode(txPin, OUTPUT);
}

/**
 * Notes:
 * Using digitalWrite() and delayMicroseconds() we will have frequent 8-12us timing errors errors
 * Using native AVR code (PORTx/_delay_us) will decrease the error rate to frequent 4us errors
 * Disabling interrupts during transmission + native AVR reduces errors to occasional 4us errors
 * With our 200+us timings even a 50us error is forgiveable so we'll just stick with digitalWrite()
 * Regarding ROM usage, it appears MicroCore's attiny13 optimizer already does the convertsion
 * and in my tests there was no difference between using digitalWrite() and native code
**/
//todo: reduce this function's size
void send(byte* data, uint8_t len){

	//we calculate the crc here, because if we do it after the transmission has started 
	//it will create a delay during transmission which causes the receiver to lose accuracy
	byte crc = crc8(data, len);

	//preamble
	//based on our pulse period our zeroes have more HIGH time than our ones
	//so we send zero bytes as preamble
	for(uint8_t i=0; i<NUM_PREAMBLE_BYTES; i++){
		transmitByte(0x00);
	}

	//start pulse
	digitalWrite(txPin, LOW);
	delayMicroseconds(START_PULSE_DURATION - HIGH_PERIOD_DURATION);
	digitalWrite(txPin, HIGH);
	delayMicroseconds(HIGH_PERIOD_DURATION - 4);	//-4 because digitalWrite takes ~4us

	//data
	for(uint8_t i=0; i<len; i++){
		transmitByte(data[i]);
	}

	///crc
	transmitByte(crc);

	//reset the line to LOW so receiver detects last pulse
	//because receiver uses falling edges to detect pulses
	digitalWrite(txPin, LOW);
	//delayMicroseconds(1000);
	//receiver relies on noise to detect end of transmission, 
	//so we send it some artificial noise to "announce" end of transmission
	//be careful choosing this because when we're here receiver is expecting a byte not a start pulse
	//so it's more sensitive
	//we really need 8 to fill the rcvdPulses[] buffer, but we send 10 just to be sure
#if !defined(EOT_IN_RX) && !defined(EOT_NONE)
	for(uint8_t i=0; i<10; i++){
		delayMicroseconds(HIGH_PERIOD_DURATION/2);
		digitalWrite(txPin, HIGH);
		delayMicroseconds(HIGH_PERIOD_DURATION/2);
		digitalWrite(txPin, LOW);
	}
#endif

}

void transmitByte(byte _byte){
	for(uint8_t i=0; i<8; i++){
		//if 1
		if(_byte & (1<<i)){
			digitalWrite(txPin, LOW);
			delayMicroseconds(ONE_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(txPin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION - 4);
		}
		else{
			digitalWrite(txPin, LOW);
			delayMicroseconds(ZERO_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(txPin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION - 4);
		}
	}
}