#include "TinyRF_RX.h"

// you can only use pins that support interrupts
// in Arduino Uno this is pins 2 and 3
int rxPin = 2;

void setup(){
	Serial.begin(9600);
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

	if(err == TRF_ERR_NO_DATA){
		return;
	}

	if(err == TRF_ERR_BUFFER_OVERFLOW){
		Serial.println("Buffer too small for received data!");
		return;
	}

	if(err == TRF_ERR_CORRUPTED){
		Serial.println("Received data corrupted.");
		return;
	}

	// this will only work if error checking and sequence numbering are enabled
	if(err == TRF_ERR_DUPLICATE_MSG){
		Serial.println("Duplicate message.");
		return;
	}

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