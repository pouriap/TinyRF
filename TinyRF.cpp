#include "Arduino.h"
#include "TinyRF.h"

//todo: everything is ruined when we change preamble even a little bit
//todo: higher speeds that should be totally possible are ruined, possibly related to above
//todo: do we need higher max error for start pulse?
//todo: when we set maximum for 1 and 0 pulses they get ruined
//todo: change pulse duration to pulse period

/*
notes:
- if we had errors when increasing data rates it's because of noise and we should enable MAX values for
pulse durations to increase accuracy
- if we had errors when increasing gap between messages it's because of preable 
*/


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
uint8_t txPin = 4;




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

void setupReceiver(uint8_t pin){
	rxPin = pin;
	pinMode(rxPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(rxPin), interrupt_routine, FALLING);
}

void setupTransmitter(uint8_t pin){
	txPin = pin;
	pinMode(txPin, OUTPUT);
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
	for(int i=0; i<PREABMLE_DURATION; i++){
		digitalWrite(txPin, LOW);
		delayMicroseconds(400);
		digitalWrite(txPin, HIGH);
		delayMicroseconds(600);
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

void transmitByte2(byte _byte){
	cli();
	for(uint8_t i=0; i<8; i++){
		//if 1
		if(_byte & (1<<i)){
			PORTB &= ~(1<<PB0);
			_delay_us(ONE_PULSE_DURATION - HIGH_PERIOD_DURATION);
			PORTB |= (1<<PB0);
			_delay_us(HIGH_PERIOD_DURATION - 4);
		}
		else{
			PORTB &= ~(1<<PB0);
			_delay_us(ZERO_PULSE_DURATION - HIGH_PERIOD_DURATION);
			PORTB |= (1<<PB0);
			_delay_us(HIGH_PERIOD_DURATION - 4);
		}
	}
	sei();
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
		Serial.print("BAD CRC: [");Serial.print((char*)buf);Serial.println("]");
		Serial.print("crcRcvd: ");Serial.print(crcRcvd, HEX);Serial.print(" crcCalc: ");Serial.print(crcCalc, HEX);Serial.print(" len: ");Serial.println(dataLen);
		return TINYRF_ERR_BAD_CRC;
	}

	return TINYRF_ERR_SUCCESS;

}

