#include "TinyRF_TX.h"

void setup(){
	// transmitter default pin is pin #2. You can change it by editing Settings.h
	setupTransmitter();
}

void loop(){
	const char* msg = "Hello from far away!";
	send((byte*)msg, strlen(msg));
	// alternatively you can provide a third argument to send a message multiple times
	// this is for reliability in case some messages get lost in the way
	// if you have error checking and sequence numbering enabled the getReceivedData() function 
	// will return TRF_ERR_DUPLICATE_MSG when receiving a duplicate, making it easy to ignore duplicates
	// it is socially more responsible to use fewer repetition to minimize your usage of the bandwidth
	// when sending multiple messages make sure you call getReceivedData() frequently in your receiver 
	// otherwise the buffer gets full
	// send((byte*)msg, strlen(msg), 10);
	delay(MIN_TX_INTERVAL);	//make sure there's at least a MIN_TX_INTERVAL delay between transmissions, otherwise the receiver's behavior will be undefined
}