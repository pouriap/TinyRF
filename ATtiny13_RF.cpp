#include "Arduino.h"
#include "ATtiny13_RF.h"

//todo: sometimes resets
//todo: sometimes there are many missed transmissions

volatile bool transmitOngoing = false;
volatile uint8_t bufIndex = 0;
byte rcvdBytes[MAX_MSG_LEN];
volatile unsigned long rcvdPulses[8];

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

void enableReceive(uint8_t pin){
	attachInterrupt(digitalPinToInterrupt(pin), interrupt_routine, FALLING);
}

bool process_received_byte(){
	byte receivedData = 0x00;
	for(uint8_t i=0; i<8; i++){
		//if pulse is greater than START_PULSE_DURATION then we will not be here
		if( rcvdPulses[i] > (ONE_PULSE_DURATION - TRIGER_ERROR) ){
			receivedData |= (1<<i);
		}
		else if( rcvdPulses[i] < (ZERO_PULSE_DURATION - TRIGER_ERROR) ){
			//this is noise
			return false;
		}
	}
	if(bufIndex >= MAX_MSG_LEN){
		Serial.println("BUFFER OVERFLOW");
		bufIndex = 0;
		return false;
	}
	else{
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
		pulseDuration > (START_PULSE_DURATION - TRIGER_ERROR)
		&& pulseDuration < (START_PULSE_DURATION + START_PULSE_MAX_ERROR)
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


void send(byte* data, uint8_t len, uint8_t pin){

	//we calculate the crc here, because if we do it after the transmission has started 
	//it will create a delay during transmission which causes the receiver to lose accuracy
	byte crc = crc8(data, len);

	//preamble
	for(uint8_t i=0; i<NUM_PREAMBLE_BYTES; i++){
		transmitByte(0x51, pin);
	}

	//start pulse
	digitalWrite(pin, LOW);
	delayMicroseconds(START_PULSE_DURATION - HIGH_PERIOD_DURATION);
	digitalWrite(pin, HIGH);
	delayMicroseconds(HIGH_PERIOD_DURATION - 4);	//-4 because digitalWrite takes ~4us

	//length
	transmitByte(len, pin);

	//data
	for(uint8_t i=0; i<len; i++){
		transmitByte(data[i], pin);
	}

	///crc
	transmitByte(crc, pin);

	//reset the line to LOW because receiver uses falling edges to detect pulses
	digitalWrite(pin, LOW);

}

void transmitByte(byte _byte, uint8_t pin){
	for(uint8_t i=0; i<8; i++){
		//if 1
		if(_byte & (1<<i)){
			digitalWrite(pin, LOW);
			delayMicroseconds(ONE_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION - 4);
		}
		else{
			digitalWrite(pin, LOW);
			delayMicroseconds(ZERO_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION - 4);
		}
	}
}

byte getReceivedData(byte buf[]){

	//return if data is not ready yet or already retreived
	if(transmitOngoing || bufIndex==0){
		return TINYRF_ERR_NO_DATA;
	}

	//copy the data
	uint8_t dataLen = rcvdBytes[0];
	for(int i=0; i<dataLen; i++){
		buf[i] = rcvdBytes[i+1];
	}

	//we got the data so reset the index
	bufIndex = 0;

	//calculate crc
	byte crcRcvd = rcvdBytes[dataLen+1];
	byte crcCalc = crc8(buf, dataLen);
	if(crcRcvd != crcCalc){
		Serial.print("BAD CRC: [");Serial.print((char*)buf);Serial.println("]");
		Serial.print("crcRcvd: ");Serial.print(crcRcvd, HEX);Serial.print(" crcCalc: ");Serial.print(crcCalc, HEX);Serial.print(" len: ");Serial.println(dataLen);
		return TINYRF_ERR_BAD_CRC;
	}

	return TINYRF_ERR_SUCCESS;

}

