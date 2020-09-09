#include "TinyRF_TX.h"

void setupTransmitter(){
	pinMode(TRF_TX_PIN, OUTPUT);
}

/**
 * Notes:
 * When the receiver is an Arduino:
 * Using digitalWrite() and delayMicroseconds() we will have frequent 8-12us timing errors
 * Using native AVR code (PORTx/_delay_us) will decrease the error rate to frequent 4us errors
 * Disabling interrupts during transmission + native AVR reduces errors to occasional 4us errors
 * With our 100+us timings even a 50us error is forgiveable so we'll just stick with digitalWrite()
 * Regarding FLASH usage, it appears MicroCore's ATtiny13 optimizer already does the convertsion
 * and in my tests there was no difference between using digitalWrite() and native code
**/
void send(byte* data, uint8_t len){

	#ifndef TRF_SEQ_DISABLED
		static uint8_t seq = 0;
	#endif

	//we calculate the crc here, because if we do it after the transmission has started 
	//it will create a delay during transmission which causes the receiver to lose accuracy
	#ifndef TRF_ERROR_CHECKING_NONE
		#ifndef TRF_SEQ_DISABLED
			byte errChck = TRF_ERR_CHK_FUNC(data, len, seq);
		#else
			byte errChck = TRF_ERR_CHK_FUNC(data, len);
		#endif
	#endif

	//preamble
	//based on our pulse period our zeroes have more HIGH time than our ones
	//so we send zero bytes as preamble
	for(uint8_t i=0; i<NUM_PREAMBLE_BYTES; i++){
		transmitByte(0x00);
	}

	//START pulse
	digitalWrite(TRF_TX_PIN, LOW);
	delayMicroseconds(START_PULSE_PERIOD - PERIOD_HIGH_DURATION);
	digitalWrite(TRF_TX_PIN, HIGH);
	delayMicroseconds(PERIOD_HIGH_DURATION - 4);	//-4 because digitalWrite takes ~4us

	//error checking byte
	#ifndef TRF_ERROR_CHECKING_NONE
	transmitByte(errChck);
	#endif

	//sequence number
	#ifndef TRF_SEQ_DISABLED
	transmitByte(seq++);
	#endif

	//data
	for(uint8_t i=0; i<len; i++){
		transmitByte(data[i]);
	}

	//reset the line to LOW so receiver detects last pulse
	//because receiver uses falling edges to detect pulses
	digitalWrite(TRF_TX_PIN, LOW);
	
	//receiver relies on noise to detect end of transmission, 
	//so we send it some artificial noise to "announce" end of transmission
	//be careful choosing this because when we're here receiver is expecting a byte not a start pulse
	//so it's more sensitive
	//we really need 8 to fill the rcvdPulses[] buffer, but we send 10 just to be sure
#if defined(TRF_EOT_IN_TX) && !defined(TRF_EOT_NONE)
	for(uint8_t i=0; i<10; i++){
		delayMicroseconds(PERIOD_HIGH_DURATION/2);
		digitalWrite(TRF_TX_PIN, HIGH);
		delayMicroseconds(PERIOD_HIGH_DURATION/2);
		digitalWrite(TRF_TX_PIN, LOW);
	}
#endif

}

void send(byte data[], uint8_t len, uint8_t times){
	for(uint8_t i=0; i<times; i++){
		send(data, len);
		delayMicroseconds(START_PULSE_PERIOD * 4);
	}
}

//sends one byte
void transmitByte(byte _byte){
	for(uint8_t i=0; i<8; i++){
		//send the LOW part of the pulse
		//the LOW duration determines wether a pulse is a 1 or a 0
		digitalWrite(TRF_TX_PIN, LOW);
		//if 1
		if(_byte & (1<<i)){
			delayMicroseconds(ONE_PULSE_PERIOD - PERIOD_HIGH_DURATION);
		}
		else{
			delayMicroseconds(ZERO_PULSE_PERIOD - PERIOD_HIGH_DURATION);
		}
		//send the HIGH part of the pulse
		//all pulses have the same HIGH duration
		digitalWrite(TRF_TX_PIN, HIGH);
		delayMicroseconds(PERIOD_HIGH_DURATION - 4);
	}
}