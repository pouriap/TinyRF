#include "TinyRF_RX.h"

// you can only use pins that support external interrupts
// in Arduino Uno this is pins 2 and 3
int rxPin = 2;

void setup(){
	Serial.begin(115200);
	setupReceiver(rxPin);
}

void loop(){

	const uint8_t bufSize = 30;
	byte buf[bufSize];
	uint8_t numLostMsgs = 0;
	uint8_t numRcvdBytes = 0;

	// number of received bytes will be put in numRcvdBytes
	// if sequence numbering is enabled the number of lost messages will be put in numLostMsgs
	// if you have disabled sequence numbering or don't need number of lost messages you can omit this argument
	uint8_t err = getReceivedData(buf, bufSize, numRcvdBytes, numLostMsgs);

	// the receiver has a 256 byte FIFO buffer
	// if getReceivedData() isn't called frequently enough then older messages will get overwritten
	// so make sure the frequency at which you send messages in your tx code is slower than the frequency
	// at which you call getReceivedData() in your rx code to prevent that
	// specially when you are using sendMulti()
	// duplicate messages that are sent using sendMulti() will stay in the buffer until you read the first one

	if(err == TRF_ERR_NO_DATA){
		return;
	}

	if(err == TRF_ERR_BUFFER_OVERFLOW){
		Serial.println("Buffer too small for received data!");
		return;
	}

	if(err == TRF_ERR_CORRUPTED){
		Serial.println("Received corrupted data.");
		return;
	}

	// if sequence numbering is enabled and you use the sendMulti() function for sending a message
	// multiple times then getReceivedData() will return TRF_ERR_SUCCESS only once
	// all the duplicate messages will be automatically ignored
	// this means all you need to do is check if the return code is TRF_ERR_SUCCESS
	// these are non-repeated, crc-valid messages
	if(err == TRF_ERR_SUCCESS){
		Serial.print("Received: ");
		for(int i=0; i<numRcvdBytes; i++){
			Serial.print((char)buf[i]);
		}
		Serial.println("");

		if(numLostMsgs>0){
			Serial.print(numLostMsgs);
			Serial.println(" messages were lost before this message.");
		}
	}
	
}