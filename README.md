# TinyRF

An Arduino library for using generic 315MHz / 433MHz RF modules with ATtiny13 as transmitter.

![433MHz / 315MHz cheap ebay RF modules](https://i.imgur.com/3bveaOp.jpg)

## Features
* Built-in error checking
* Built-in sequence numbering
* Data transmission speeds up to 1000bps (2500bps with calibrated clock)
* Up to 255 byte message length
* Ability to disable features in order to preserve memory space
* Relatively small memory footprint (~300bytes FLASH, 5bytes RAM with all features enabled)

**Transmitter MCU support:** ATtiny13 or any other AVR microcontroller you can program with Arduino IDE.

**Receiver MCU support:** Currently a more capable board like an Arduino is needed for receiver because the receiver part needs more resources.

**Arduino IDE support:** Arduino IDE 1.6.0 and higher.

## How to use
Copy the contents of the repository in your Arduino libraries folder in a subfolder called `TinyRF`.

### Transmitter sketch:
```
#include "TinyRF_TX.h"

void setup(){
	//Transmitter default pin is pin #2. You can change it by editing Settings.h
	setupTransmitter();
}

void loop(){
	const char* msg = "Hello from far away!";
	send((byte*)msg, strlen(msg));
	//alternatively you can provide a third argument to send a message multiple times
	//this is for reliability in case some messages get lost in the way
	//if you have error checking and sequence numbering enabled the getReceivedData() function will return TINYRF_ERR_DUPLICATE_MSG when receiving a duplicate, making it easy to ignore duplicates
	//it is socially more responsible to use fewer repetition to minimize your usage of the bandwidth
	//send((byte*)msg, strlen(msg), 10);
	delay(1000);	//make sure there's at least a 10ms delay between transmissions
}
```

### Receiver sketch:
```
#include "TinyRF_RX.h"

int rxPin = 2;

void setup(){
	Serial.begin(9600);
	setupReceiver(rxPin);
}

void loop(){

	const unsigned int bufSize = 30;
	byte buf[bufSize];
	unsigned char numLostMsgs = 0;
	unsigned char numRcvdBytes = 0;
	//number of received bytes will be put in numRcvdBytes
	//if sequence numbering is enabled the number of lost messages will be put in numLostMsgs
	//if you have disabled sequence numbering or don't need number of lost messages you can omit this argument
	int err = getReceivedData(buf, bufSize, numRcvdBytes, numLostMsgs);

	if(err == TINYRF_ERR_NO_DATA){
		return;
	}

	if(err == TINYRF_ERR_BUFFER_OVERFLOW){
		Serial.println("Buffer too small for received data!");
		return;
	}

	if(err == TINYRF_ERR_BAD_CRC){
		Serial.println("Received data curropted.");
		return;
	}

	//this will only work if error checking and sequence numbering are enabled
	if(err == TINYRF_ERR_DUPLICATE_MSG){
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
```

## How to change settings:
Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. To find out which settings are available and what they do take a look at `Settings.h`. 

## Why did you make this library:
These generic 315MHz / 433MHz RF modules are cheap. The ATtiny13 is cheap. Why not use them together to make a cheap yet reliable system? 

With the built-in error checking and sequence numbering you can make sure your packets arrive untouched. This is useful when you need to send small amounts of data using the least amount of power and resources. For example an station that measures soil moisture on regular intervals and sends it to the central receiver. I couldn't find anything library that was both reliable and small enough to fit in an ATtiny, so I wrote one.