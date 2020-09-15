#include "TinyRF_RX.h"


/*
notes:
- if we had errors when increasing data rates it's because of noise and we should enable MAX values for
pulse periods to increase accuracy
- if we had errors when increasing gap between messages it's because of preable 
- sometimes preamble can act as EOT, this should not be relied on and should be prevented
- we send zero bytes as preamble so EOT HAS TO be detected before next transmission begins
other wise these zeroes will be considered part of the previous message
- it's possible that START pulse can act as EOT but this should not be used 
*/


namespace tinyrf{
	
	//is true when START is received but EOT hasn't happened yet
	volatile bool transmitOngoing = false;
	//set to true every time interrupt runs, used to determine when data hasn't come in for a long time
	volatile bool interruptRun = false;
	//efficient alternative to attachInterrupt/detachInterrupt
	volatile bool interruptDisabled = false;
	//index of rcvdBytsBuf to put the next byte in
	volatile uint8_t bufIndex = 0;
	//buffer for received bytes
	volatile byte rcvdBytsBuf[TRF_RX_BUFFER_SIZE];
	//buffer for received pulses(bits)
	volatile unsigned long rcvdPulses[8];
	volatile uint8_t numMsgsInBuffer = 0;
	//beggining of the current message in buffer, the value of this will be the length of the message
	volatile uint8_t msgAddrInBuf = 0;
	//length of the current message
	volatile uint8_t frameLen = 0;
	//pin used for transmission, should support external interrupts
	uint8_t rxPin = 2;

}


void setupReceiver(uint8_t pin){
	using namespace tinyrf;
	rxPin = pin;
	pinMode(rxPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(rxPin), interrupt_routine, FALLING);
}


/**
 * This function is called when end of transmission is detected either through interrupt or 
 * getReceivedData()
**/
inline void EOT(){
	using namespace tinyrf;
	transmitOngoing = false;
	//the transmission has ended
	//put the message length at the beggining of the message data in buffer
	//increment numMsgsInBuffer
	//increment bufIndex
	if(frameLen>0){
		rcvdBytsBuf[msgAddrInBuf] = frameLen;
		numMsgsInBuffer++;
		//if a message's length is 0, then this block will not run and bufIndex will stay
		//the same and next msg will be written over it
		bufIndex++;
	}
}


/**
 * This function is called from the interrupt routine when 8 bits of data has been received
 * It turns the bits into a byte and puts it in the buffer
**/
inline void process_received_byte(){
	using namespace tinyrf;
	byte receivedData = 0x00;
	for(uint8_t i=0; i<8; i++){
		//if pulse is greater than START_PULSE_PERIOD then we will not be here
		if( 
			rcvdPulses[i] > (ONE_PULSE_PERIOD - ONE_PULSE_TRIGG_ERROR)
			&& rcvdPulses[i] < (ONE_PULSE_PERIOD + ONE_PULSE_TRIGG_ERROR)
		){
			receivedData |= (1<<i);
		}
		else if( 
			rcvdPulses[i] < (ZERO_PULSE_PERIOD - ZERO_PULSE_TRIGG_ERROR)
			|| rcvdPulses[i] > (ZERO_PULSE_PERIOD + ZERO_PULSE_TRIGG_ERROR)
		){
			//this is noise = end of transmission
			EOT();
			return;
		}
	}

	//TRF_PRINT((char)receivedData);

	//we have received one bytes of data
	//add it to the buffer
	//increment bufIndex
	//increment frameLen
	rcvdBytsBuf[++bufIndex] = receivedData;
	frameLen++;
	return;

}


/**
 * Interrupt routine called on falling edges of pulses
 * We use a pulse period encoding to determine what a pulse means
 * As suggested here: http://www.romanblack.com/RF/cheapRFmodules.htm
 * This way we will have both HIGH and LOW periods in both 1 and 0 bits, eliminating 
 * the need for manchester encoding and other workarounds for sending equal HIGH and LOW
 * Pulse periods are defined in the file TinyRF.h
 * This interrupt routine usually take 8us - sometimes goes up to 30us
 * With our 100+us pulse periods this shouldn't be a problem
**/
void interrupt_routine(){

	using namespace tinyrf;

	interruptRun = true;

	static uint8_t pulse_count = 0;
	static unsigned long lastTime = 0;

	unsigned long time = micros();
	unsigned long pulsePeriod = time - lastTime;
	lastTime = time;

	if(interruptDisabled){
		return;
	}

	//TRF_PRINTLN(pulsePeriod);
	
	//start of transmission
	if( 
		pulsePeriod > (START_PULSE_PERIOD - START_PULSE_TRIGG_ERROR)
		&& pulsePeriod < (START_PULSE_PERIOD + START_PULSE_MAX_ERROR)
	){
		//if we receive a start while we are already processing an ongoing transmission
		//it means the previous transmission has ended
		if(transmitOngoing){
			EOT();
		}
		transmitOngoing = true;
		pulse_count = 0;
		msgAddrInBuf = bufIndex;
		frameLen = 0;
	}
	else if(transmitOngoing){
		rcvdPulses[pulse_count] = pulsePeriod;
		pulse_count++;
	}

	if(pulse_count == 8){
		process_received_byte();
		pulse_count = 0;
	}

	//TRF_PRINTLN(micros() - time);

}


uint8_t getReceivedData(byte buf[], uint8_t bufSize, uint8_t &numRcvdBytes, uint8_t &numLostMsgs){

	using namespace tinyrf;

	//indicates where in the received bytes buffer to read next, used by getReceivedData()
	static uint8_t bufferReadIndex = 0;

	numRcvdBytes = 0;
	numLostMsgs = 0;

	//we rely on noise to detect end of transmission
	//in the rare event that there was no noise(the interrupt did not trigger) for a long time
	//consider the transmission over and add received data to buffer
#if !defined(TRF_EOT_IN_TX) && !defined(TRF_EOT_NONE)

	//todo: there is no guarantee this will be called frequently enough so put it in a timer

	//we can't use lastTime which is set in the interrupt because it is a long (non-atomic)
	//and using it here will wreak havoc, so we calculate another one here
	//it does not matter that it takes a bit longer because this function is called from loop
	//not from interrupt
	static unsigned long lastInterruptRun = 0;
	unsigned long time = micros();
	if(interruptRun){
		lastInterruptRun = time;
		interruptRun = false;
	}
	else if(transmitOngoing){
		//actually instead of being > START_PULSE_PERIOD, it should be > ONE_PULSE_PERIOD
		//because after transmission has started we only expect 1 and 0, and if we haven't
		//received a byte for more than ONE_PULSE_PERIOD it means transmission is over
		//but for some reason using that value causes lost messages and I cannot for the life of
		//me find out why
		if( (time - lastInterruptRun) > (MIN_TX_INTERVAL_REAL) ){
			//we don't want the interrupt to run while we're modifying these
			//it's unlikely that this will hurt the interrupt because if it hasn't run for a while
			//it means transmission has stopped. also this is quite short
			interruptDisabled = true;
			EOT();
			interruptDisabled = false;
		}
	}

#endif

	if(numMsgsInBuffer == 0){
		return TRF_ERR_NO_DATA;
	}

	// TRF_PRINT("len addr: ");TRF_PRINT2(bufferReadIndex, DEC);
	// TRF_PRINT(" - #msgs in buf: ");TRF_PRINT(numMsgsInBuffer);

	/* manage buffer */
	//this is how our buffer looks like:
	//[frm0 len|frm0 crc|frm0 seq#|frm0 byte0|frm0 byte1|...|frm1 len|frm1 crc|frm1 seq#|frm1 byte0|frm1 byte1|...]
	//frame length = data length + seq# + error checking byte
	//bufferReadIndex points to the first byte of frame, i.e. the length
	uint8_t frameLen = rcvdBytsBuf[bufferReadIndex];
	bufferReadIndex++;
	uint8_t frameReadIndex = bufferReadIndex;
	//move bufferReadIndex 'length' bytes forward to point to the next frame
	//we need extra calculation to account for resetting of the 8bit buffer index
	bufferReadIndex += frameLen;
	//we consider this message processed as of now
	numMsgsInBuffer--;
	//a frame's minimum length is 3 bytes: CRC + SEQ + 1 Byte Data (at least)
	if(frameLen == 0){
		return TRF_ERR_NO_DATA;
	}
	else if(frameLen < 3){
		return TRF_ERR_NOISE;
	}

	uint8_t dataLen = frameLen;

	#ifndef TRF_ERROR_CHECKING_NONE
		dataLen--;
		byte errChckRcvd = rcvdBytsBuf[frameReadIndex];
		frameReadIndex++;
	#endif

	#ifndef TRF_SEQ_DISABLED
		dataLen--;
		uint8_t seq = rcvdBytsBuf[frameReadIndex];
		frameReadIndex++;
	#endif

	numRcvdBytes = dataLen;

	if(dataLen > bufSize){
		return TRF_ERR_BUFFER_OVERFLOW;
	}

	// TRF_PRINT(" - read index: ");TRF_PRINT2(frameReadIndex, DEC);
	// TRF_PRINT(" - len: ");TRF_PRINT(frameLen);
	// TRF_PRINTLN("");
	// for(int i=0; i<256; i++){
	// 	TRF_PRINT(i);TRF_PRINT("[");TRF_PRINT(rcvdBytsBuf[i]);TRF_PRINT("],");
	// }
	// TRF_PRINTLN("");

	//copy the data from 'frameReadIndex' until frameReadIndex+dataLen
	for(uint8_t i=0; i<dataLen; i++){
		buf[i] = rcvdBytsBuf[frameReadIndex++];
	}

	//error checking
	#ifndef TRF_ERROR_CHECKING_NONE
		#ifndef TRF_SEQ_DISABLED
			byte errChckCalc = TRF_ERR_CHK_FUNC(buf, dataLen, seq);
		#else
			byte errChckCalc = TRF_ERR_CHK_FUNC(buf, dataLen);
		#endif
		if(errChckRcvd != errChckCalc){
			return TRF_ERR_CORRUPTED;
		}
		//if all data is zeroes CRC will also be zero and CRC check will pass
		//we can eliminate this by using a non-zero crc init but that would increase crc function 
		//size, so we do this here in the RX instead
		else if( (errChckRcvd|errChckCalc) == 0){
			boolean allZeroes = true;
			for(uint8_t i=0; i<dataLen; i++){
				if(buf[i] != 0x00){
					allZeroes = false;
				}
			}
			if(allZeroes){
				return TRF_ERR_NOISE;
			}
		}
	#endif

	//sequence number
	#ifndef TRF_SEQ_DISABLED

		static int lastSeq = -2;

		//if this is the first seq we receive
		if(lastSeq == -2){
			lastSeq = seq;
			//TRF_PRINT(seq);TRF_PRINT(":");
			return TRF_ERR_SUCCESS;
		}
		else if(seq == lastSeq){
			//we can only rely on seq# for detecting duplicates if we have error checking
			#ifndef TRF_ERROR_CHECKING_NONE
				return TRF_ERR_DUPLICATE_MSG;
			#else
				return TRF_ERR_SUCCESS;
			#endif
		}

		if(seq > lastSeq+1){
			numLostMsgs = seq - lastSeq - 1;
		}
		else{
			//seq is smaller than lastseq meaning seq was reset during lost messages
			numLostMsgs = 255 - lastSeq + seq;
		}

		//TRF_PRINT(seq);TRF_PRINT(":");

		if(seq == 255){
			//because next valid seq will be 0 
			lastSeq = -1;
		}
		else{
			lastSeq = seq;
		}

	#endif

	return TRF_ERR_SUCCESS;

}

uint8_t getReceivedData(byte buf[],  uint8_t bufSize, uint8_t &numRcvdBytes){
	uint8_t l = 0;
	return getReceivedData(buf, bufSize, numRcvdBytes, l);
}