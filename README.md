# TinyRF

An Arduino library for using generic 315MHz / 433MHz RF modules with ATtiny13 as transmitter.

![433MHz / 315MHz cheap ebay RF modules](https://repository-images.githubusercontent.com/293609741/4b910480-f297-11ea-96e6-fd41628b4086)

## Features
* Built-in error checking
* Built-in sequence numbering
* Data transmission speeds up to 1000bps (2500bps with calibrated clock)
* Ability to disable features in order to preserve memory space
* Small memory usage: 172 Bytes FLASH, 1 Byte RAM with all features enabled. (Using [MicroCore](https://github.com/MCUdude/MicroCore) with LTO enabled)

**Transmitter MCU support:** ATtiny13 or any other AVR microcontroller you can program with Arduino IDE.

**Receiver MCU support:** Currently a more capable board like an Arduino is needed for receiver because the receiver part needs more resources.

**Arduino IDE support:** Arduino IDE 1.6.0 and higher.

## How to use
Copy the contents of the repository in your Arduino libraries folder in a subfolder called `TinyRF`.

### Transmitter sketch:
```C++
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
	//if you have error checking and sequence numbering enabled the getReceivedData() function will return TRF_ERR_DUPLICATE_MSG when receiving a duplicate, making it easy to ignore duplicates
	//it is socially more responsible to use fewer repetition to minimize your usage of the bandwidth
	//send((byte*)msg, strlen(msg), 10);
	delay(1000);	//make sure there's at least a 10ms delay between transmissions
}
```

### Receiver sketch:
```C++
#include "TinyRF_RX.h"

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
	//number of received bytes will be put in numRcvdBytes
	//if sequence numbering is enabled the number of lost messages will be put in numLostMsgs
	//if you have disabled sequence numbering or don't need number of lost messages you can omit this argument
	uint8_t err = getReceivedData(buf, bufSize, numRcvdBytes, numLostMsgs);

	if(err == TRF_ERR_NO_DATA){
		return;
	}

	if(err == TRF_ERR_BUFFER_OVERFLOW){
		Serial.println("Buffer too small for received data!");
		return;
	}

	if(err == TRF_ERR_BAD_CRC){
		Serial.println("Received data curropted.");
		return;
	}

	//this will only work if error checking and sequence numbering are enabled
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
```

## How to reduce memory usage?
If you're running out of memory here are a few TinyRF-specific and general hints:
* Disable sequence numbering: If you don't need the sequence numbering feature you can disable it by uncommenting `#define TRF_SEQ_DISABLED` in `Settings.h`. Note that disabling sequence numbering will also disable TinyRF's ability to detect duplicate messages and the `TRF_ERR_DUPLICATE_MSG` return code.
* Use checksum for error detection: Checksum detects less errors compared to CRC (the default error checking) but also occupies less memory. To use checksum instead of CRC uncomment `#define TRF_ERROR_CHECKING_CHECKSUM` in `Settings.h` and comment out the CRC define.
* Disable error checking altogether: If you don't need the error checking you can completely disable it by uncommenting `#define TRF_ERROR_CHECKING_NONE` in `Settings.h`. You should also comment checksum and CRC.
* Checkout [Efficient C Coding for AVR](https://teslabs.com/openplayer/docs/docs/prognotes/efficient_c_coding_avr.pdf). Specially the section about "Variables and Data
Types".
* Use Atmel Studio and write in pure C instead of using Arduino. There is some overhead when using even the `setup()` and `loop()` function so if you need every last byte you should forget Arduino libraries and just use purce C.

## How to use without Arduino?
In order to make the library easy to use for Arduino users I have written the library with Arduino functions such as `digitalWrite()` and `delayMicroseconds()`. The optimizer automatically takes care of them and they don't add an overhead (tested with ATtiny13 + MicroCore). If you want to use it in a pure C AVR project you'll have to replace the functions yourself.

## How to change settings:
Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. To find out which settings are available and what they do take a look at `Settings.h`. 