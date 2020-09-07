#include "TinyRF_RX.h"

volatile bool transmitOngoing = false;
volatile bool interruptRun = false;
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

/**
 * This function is called from the interrupt routine when 8 bits of data has been received
 * It turns the bits into a byte and puts it in the buffer
**/
inline void process_received_byte(){
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

			//the transmission has ended
			//put the message length at the beggining of the message data in buffer
			//increate numMsgsInBuffer
			//increment bufIndex for holding the next message
			if(msgLen>0){
				rcvdBytsBuf[msgAddrInBuf] = msgLen;
				numMsgsInBuffer++;
				//if a message's length is 0, then this block will not run and bufIndex will stay
				//the same and next msg will be written over it
				bufIndex++;
			}

			return;
		}
	}

	//Serial.print((char)receivedData);

	//we have received one bytes of data
	//add it to the buffer
	//increment bufIndex
	//increment msgLen
	rcvdBytsBuf[++bufIndex] = receivedData;
	msgLen++;
	return;

}

/**
 * Interrupt routine called on falling edges of pulses
 * We measure a pulse's period to determine wheter it is a START, 1 or 0 pulse
 * this interrupt routine usually take 8us - sometimes goes up to 30us
 * with our 100+us pulse durations this shouldn't be a problem
**/
void interrupt_routine(){

	interruptRun = true;

	static uint8_t pulse_count = 0;

	unsigned long time = micros();
	unsigned long pulsePeriod = time - lastTime;
	lastTime = time;

	//Serial.println(pulseDuration);
	
	//start of transmission
	if( 
		pulsePeriod > (START_PULSE_DURATION - TRIGER_ERROR)
		&& pulsePeriod < (START_PULSE_DURATION + START_PULSE_MAX_ERROR)
	){
		//if we receive a start while we are already processing an ongoing transmission
		//we add the length of the previous message before starting to process this one
		//this prevents a curropted buffer where length will be zero because transmission was never finished
		//todo: or perhaps we should just ignore these messages because they're useless
		if(transmitOngoing){
			rcvdBytsBuf[msgAddrInBuf] = msgLen;
			numMsgsInBuffer++;
			bufIndex++;
		}
		transmitOngoing = true;
		pulse_count = 0;
		msgAddrInBuf = bufIndex;
		msgLen = 0;
	}
	else if(transmitOngoing){
		rcvdPulses[pulse_count] = pulsePeriod;
		pulse_count++;
	}

	if(pulse_count == 8){
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
#if !defined(EOT_IN_TX) && !defined(EOT_NONE)

	static unsigned long lastInterruptRun = 0;
	unsigned long time = micros();
	if(interruptRun){
		lastInterruptRun = time;
		interruptRun = false;
	}
	else if(transmitOngoing){
		if( (time - lastInterruptRun) > (START_PULSE_DURATION*2) ){
			//we don't want the interrupt to run while we're modifying these
			//it's unlikely that this will hurt the interrupt because if it hasn't run for a while it means
			//transmission has stopped. also this is quite short
			detachInterrupt(digitalPinToInterrupt(rxPin));
			transmitOngoing = false;
			rcvdBytsBuf[msgAddrInBuf] = msgLen;
			numMsgsInBuffer++;
			bufIndex++;
			attachInterrupt(digitalPinToInterrupt(rxPin), interrupt_routine, FALLING);
		}
	}

#endif

	if(numMsgsInBuffer == 0){
		return TINYRF_ERR_NO_DATA;
	}

	//Serial.print("len addr: ");Serial.print(bufferReadIndex, DEC);
	//Serial.print(" - #msgs in buf: ");Serial.print(numMsgsInBuffer);

	/* manage buffer */
	//this is how our buffer looks like:
	//[msg0 len|msg0 byte0|msg0 byte1|...|msg0 crc|msg1 len|msg1 byte0|msg1 byte1|msg1 crc|...]
	//message length = data length + error checking byte
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

	//error checking
	#ifndef ERROR_CHECKING_NONE
		byte errChckRcvd = rcvdBytsBuf[bufferReadIndex];
		//move buffer pointer to next byte
		bufferReadIndex++;
		byte errChckCalc = ERROR_CHECKING(buf, dataLen);

		//Serial.print("bufferReadIndex is now: ");Serial.println(bufferReadIndex, DEC);

		if(errChckRcvd != errChckCalc){
			//Serial.print("BAD CRC: [");Serial.print((char*)buf);Serial.println("]");
			//Serial.print("crcRcvd: ");Serial.print(errChckRcvd, HEX);Serial.print(" crcCalc: ");Serial.print(errChckCalc, HEX);Serial.print(" len: ");Serial.println(dataLen);
			return TINYRF_ERR_BAD_CRC;
		}
	#endif

	return TINYRF_ERR_SUCCESS;

}
