#include "TinyRF_RX.h"

volatile bool transmitOngoing = false;
volatile uint8_t bufIndex = 0;
byte rcvdBytsBuf[RX_BUFFER_SIZE];
volatile unsigned long rcvdPulses[8];
volatile uint8_t numMsgsInBuffer = 0;
volatile uint8_t msgAddrInBuf = 0;
volatile uint8_t msgLen = 0;
uint8_t bufferReadIndex = 0;
volatile unsigned long lastTime = 0;
uint8_t rxPin = 2;


void setupReceiver(uint8_t pin){
	rxPin = pin;
	pinMode(rxPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(rxPin), interrupt_routine, FALLING);
}


inline bool process_received_byte(){
	byte receivedData = 0x00;
	for(uint8_t i=0; i<8; i++){
		//if pulse is greater than START_PULSE_DURATION then we will not be here
		if( 
			rcvdPulses[i] > (ONE_PULSE_DURATION - TRIGER_ERROR)
			&& rcvdPulses[i] < (ONE_PULSE_DURATION + TRIGER_ERROR)
		){
			receivedData |= (1<<i);
		}
		else if( 
			rcvdPulses[i] < (ZERO_PULSE_DURATION - TRIGER_ERROR)
			|| rcvdPulses[i] > (ZERO_PULSE_DURATION + TRIGER_ERROR)
		){
			//this is noise = end of transmission
			transmitOngoing = false;

			//Serial.println("");

			if(msgLen>0){
				rcvdBytsBuf[msgAddrInBuf] = msgLen;
				numMsgsInBuffer++;
				//if a message's length is 0, then this block will not run and bufIndex will stay
				//the same and next msg will be written over it
				bufIndex++;
			}

			return false;
		}
	}

	//Serial.print((char)receivedData);

	rcvdBytsBuf[++bufIndex] = receivedData;
	msgLen++;
	return true;

}

//this interrupt routine usually take 8us - sometimes goes up to 30us
//with our 100+us pulse durations this shouldn't be a problem
void interrupt_routine(){

	static uint8_t pulse_count = 0;

	unsigned long time = micros();
	unsigned long pulseDuration = time - lastTime;
	lastTime = time;

	//Serial.println(pulseDuration);
	
	//start of transmission
	//we also check the maximum duration for start pulse so that our start detection will be more accurate 
	if( 
		pulseDuration > (START_PULSE_DURATION - TRIGER_ERROR)
		&& pulseDuration < (START_PULSE_DURATION + START_PULSE_MAX_ERROR)
	){
		//if we receive a start while we are already processing an ongoing transmission
		//we add the length of the previous message before starting to process this one
		//this prevents a curropted buffer where length will be zero because transmission was
		//never finished, and after that zero there will be some bytes that the getReceivedData()
		//will treat as length
		//todo: or perhaps we should just ignore these messages because they're useless
		if(transmitOngoing){
			rcvdBytsBuf[msgAddrInBuf] = msgLen;
			numMsgsInBuffer++;
			//if a message's length is 0, then this block will not run and bufIndex will stay
			//the same and next msg will be written over it
			bufIndex++;
		}
		transmitOngoing = true;
		pulse_count = 0;
		msgAddrInBuf = bufIndex;
		msgLen = 0;
	}
	else if(transmitOngoing){
		rcvdPulses[pulse_count] = pulseDuration;
		pulse_count++;
	}

	if(pulse_count == 8){
		//reset if received bad byte (i.e noise = end of transmission)
		//todo: this doesn't need return type
		process_received_byte();
		pulse_count = 0;
	}

	//Serial.println(micros() - time);

}



byte getReceivedData(byte buf[]){

	//we rely on noise to detect end of transmission
	//in the rare event that there was no noise(the interrupt did not trigger) for a long time
	//consider the transmission over and add received data to buffer
#if defined(EOT_IN_RX) && !defined(EOT_NONE)
	if(transmitOngoing){
		//we need to detach the interrupt because lastTime is non-atomic
		detachInterrupt(digitalPinToInterrupt(rxPin));
		if( 
			(msgLen > 0) && 
			((micros() - lastTime) > START_PULSE_DURATION + START_PULSE_MAX_ERROR) 
		){
			transmitOngoing = false;
			rcvdBytsBuf[msgAddrInBuf] = msgLen;
			numMsgsInBuffer++;
			bufIndex++;
		}
		attachInterrupt(digitalPinToInterrupt(rxPin), interrupt_routine, FALLING);
	}
#endif

	if(numMsgsInBuffer == 0){
		return TINYRF_ERR_NO_DATA;
	}

	//Serial.print("len addr: ");Serial.print(bufferReadIndex, DEC);
	//Serial.print(" - #msgs in buf: ");Serial.print(numMsgsInBuffer);

	/* manage buffer */
	//message length = data length + crc byte
	//bufferReadIndex points to the first byte of message, i.e. the length
	uint8_t msgLen = rcvdBytsBuf[bufferReadIndex];
	//move buffer pointer one byte further
	//after this bufferReadIndex points to the first byte of the actual data
	bufferReadIndex++;
	//we consider this message processed as of now
	numMsgsInBuffer--;
	//if message's length is zero then we don't need to do anything more
	if(msgLen == 0){
		return TINYRF_ERR_NO_DATA;
	}

	//dataLen is the actual data, i.e. minus the CRC
	uint8_t dataLen = msgLen - 1;

	//Serial.print(" - read index: ");Serial.print(bufferReadIndex, DEC);
	//Serial.print(" - len: ");Serial.print(msgLen);
	//Serial.println("");
	//for(int i=0; i<256; i++){
	//	Serial.print(rcvdBytsBuf[i]);Serial.print(",");
	//}
	//Serial.println("");

	//copy the data from 'bufferReadIndex' until bufferReadIndex+dataLen
	for(int i=0; i<dataLen; i++){
		buf[i] = rcvdBytsBuf[bufferReadIndex++];
	}

	//crc
	byte crcRcvd = rcvdBytsBuf[bufferReadIndex];
	//go to next byte
	bufferReadIndex++;
	byte crcCalc = crc8(buf, dataLen);

	//Serial.print("bufferReadIndex is now: ");Serial.println(bufferReadIndex, DEC);

	if(crcRcvd != crcCalc){
		#ifndef __AVR_ATtiny13__
		Serial.print("BAD CRC: [");Serial.print((char*)buf);Serial.println("]");
		Serial.print("crcRcvd: ");Serial.print(crcRcvd, HEX);Serial.print(" crcCalc: ");Serial.print(crcCalc, HEX);Serial.print(" len: ");Serial.println(dataLen);
		#endif
		return TINYRF_ERR_BAD_CRC;
	}

	return TINYRF_ERR_SUCCESS;

}
