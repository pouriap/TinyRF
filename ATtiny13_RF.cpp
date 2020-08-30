#include "Arduino.h"
#include "ATtiny13_RF.h"

/*
const unsigned int START_PULSE_DURATION = 8000;
const unsigned int START_TRIGGER = 7950;
const unsigned int ONE_PULSE_DURATION = 5000;
const unsigned int ONE_TRIGGER = 4950;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int ZERO_TRIGGER = 2950;
const unsigned int HIGH_PERIOD_DURATION = 2000;
const int TRIGER_ERROR = 50;
*/

const unsigned int START_PULSE_DURATION = 2000;
const unsigned int ONE_PULSE_DURATION = 1000;
const unsigned int ZERO_PULSE_DURATION = 800;
const unsigned int HIGH_PERIOD_DURATION = 500;
const int TRIGER_ERROR = 50;
const int START_PULSE_MAX_ERROR = 400;

/*
const unsigned int START_PULSE_DURATION = 3000;
const unsigned int START_TRIGGER = 2950;
const unsigned int ONE_PULSE_DURATION = 2000;
const unsigned int ONE_TRIGGER = 1950;
const unsigned int ZERO_PULSE_DURATION = 1500;
const unsigned int ZERO_TRIGGER = 1450;
const unsigned int HIGH_PERIOD_DURATION = 1000;
const int TRIGER_ERROR = 50;
*/

volatile bool transmitOngoing = false;
const int MAX_MSG_LEN = 16;
volatile uint8_t bufIndex = 0;
char rcvdBytes[MAX_MSG_LEN];
volatile unsigned long rcvdPulses[8];


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

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
uint8_t crc8(uint8_t data[], uint8_t len){
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


void enableReceive(uint8_t pin){
	attachInterrupt(digitalPinToInterrupt(pin), interrupt_routine, FALLING);
}

bool process_received_byte(){
	uint8_t receivedData = 0x00;
	for(int i=0; i<8; i++){
		//if pulse is greater than START_PULSE_DURATION then we will not be here
		if( rcvdPulses[i] > (ONE_PULSE_DURATION - TRIGER_ERROR) ){
			receivedData |= (1<<i);
		}
		else if( rcvdPulses[i] < (ZERO_PULSE_DURATION - TRIGER_ERROR) ){
			//curropted
			return false;
		}
	}
	if(bufIndex>=MAX_MSG_LEN){
		Serial.println("BUFFER OVERFLOW");
		bufIndex = 0;
		return false;
	}
	else{
		//Serial.print("r:");Serial.println(receivedData);
		rcvdBytes[bufIndex++] = receivedData;
		return true;
	}
}

void interrupt_routine(){

	static unsigned long lastTime = 0;
	static unsigned int pulse_count = 0;

	unsigned long time = micros();
	unsigned long pulseDuration = time - lastTime;
	lastTime = time;
	
	//start of transmission
	//we also check the maximum duration for start pulse so that our start detection will be more accurate 
	if( 
		pulseDuration > (START_PULSE_DURATION - TRIGER_ERROR) && 
		pulseDuration < (START_PULSE_DURATION + START_PULSE_MAX_ERROR) 
	){
		transmitOngoing = true;
		pulse_count = 0;
	}
	else if(transmitOngoing){
		rcvdPulses[pulse_count] = pulseDuration;
		pulse_count++;
	}

	if(pulse_count == 8){
		//reset if received bad byte (i.e noise = end of transmission)
		if(!process_received_byte()){
			transmitOngoing = false;
		}
		pulse_count = 0;
	}

}


void send(char* data, uint8_t len, uint8_t pin){

	//premeable
	//for(uint8_t i=0; i<64; i++){
	//	digitalWrite(pin, LOW);
	//	delayMicroseconds(1000);
	//	digitalWrite(pin, HIGH);
	//	delayMicroseconds(1000);
	//}
	for(int i=0; i<4; i++){
		transmitByte(0x51, pin);
	}

	//start pulse
	digitalWrite(pin, LOW);
	delayMicroseconds(START_PULSE_DURATION - HIGH_PERIOD_DURATION);
	digitalWrite(pin, HIGH);
	delayMicroseconds(HIGH_PERIOD_DURATION-4);	//-4 because digitalWrite takes 3.6us

	//len
	transmitByte(len, pin);

	//data
	for(uint8_t i=0; i<len; i++){
		transmitByte(data[i], pin);
	}

	///crc
	uint8_t crc = crc8(data, len);
	transmitByte(crc, pin);
}

void transmitByte(char byte, uint8_t pin){
	for(uint8_t i=0; i<8; i++){
		//if 1
		if(byte & (1<<i)){
			digitalWrite(pin, LOW);
			delayMicroseconds(ONE_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION-4);
		}
		else{
			digitalWrite(pin, LOW);
			delayMicroseconds(ZERO_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION-4);
		}
		digitalWrite(pin, LOW);
	}
}

uint8_t getReceivedData(uint8_t buf[]){
	//return if data is not ready yet or already retreived
	if(transmitOngoing || bufIndex==0){
		return TINYRF_ERR_NO_DATA;
	}
	//copy the data
	int dataLen = rcvdBytes[0];
	for(int i=0; i<dataLen; i++){
		buf[i] = rcvdBytes[i+1];
	}
	//we got the data so reset the index
	bufIndex = 0;
	//calculate crc
	byte crcRcvd = rcvdBytes[dataLen+1];
	//todo: add len to crc as well
	byte crcCalc = crc8(buf, dataLen);
	if(crcRcvd != crcCalc){
		return TINYRF_ERR_BAD_CRC;
	}
	return TINYRF_ERR_SUCCESS;
}

