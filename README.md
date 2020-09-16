# TinyRF

A small Arduino library for generic 315MHz / 433MHz RF modules.

The transmitter code is small in size making it suitable for microcontrollers with limited FLASH space. Namely the ATtiny13, but Arduino and other AVR MCUs are also supported.

![433MHz / 315MHz cheap ebay RF modules](https://repository-images.githubusercontent.com/293609741/4b910480-f297-11ea-96e6-fd41628b4086)

## Features
* Built-in error checking
* Built-in sequence numbering
* Data transmission speeds up to 1000bps (2500bps with calibrated clock)
* Ability to disable features in order to preserve memory space
* Small(ish) memory usage: 192 Bytes FLASH, 1 Byte RAM with all features enabled. (Using [MicroCore](https://github.com/MCUdude/MicroCore) with LTO enabled)

**Transmitter MCU support:** ATtiny13 or any other AVR microcontroller you can program with Arduino IDE.

**Receiver MCU support:** Arduino is recommended. But any other AVR microcontroller that runs at 16MHz cpu speed or higher and has at least one external interrupt pin with sufficient FLASH memory (>4KB) should work. 

**Arduino IDE support:** Arduino IDE 1.6.0 and higher.

## How to install the library
Download the [latest release](https://github.com/pouriap/TinyRF/releases/latest) and copy the contents in your Arduino "libraries" folder.

## Usage notes:
* The internal clock(s) of the ATtiny13 can be inaccurate. Specially the 4.8MHz oscillator because by default only the calibration data for the 9.6MHz oscillator is copied. I highly recommend that you [calibrate your chip](https://github.com/MCUdude/MicroCore#internal-oscillator-calibration) to get more accurate timings. The library might not even work depending on how inaccurate your chip is.
* Make sure you call `getReceivedData()` as frequently as possible in your receiver sketch loop.
* You can technically send messages as long as 250 Bytes long but that is not recommended. The longer your messages are the more susceptible to noise they become. Also the error checking byte will detect less and less errors the longer your message is.
* Don't use Arduino IDE's "include library" feature, as it will include unnecessary files. Just include "TinyRF_TX.h" or "TinyRF_RX.h" at the beggining of your sketch.
* Documentation is provided in form of comments in the example transmitter and receiver sketches.
* Check out `Settings.h` to find out which settings are available and what they do.
* Don't forget proper powering! A 0.1uF decoupling cap for the MCU is **mandatory**. I also highly recommend an additional 10/22uF. Also use a nice and stable power source. This will minimize errors and headaches.

### Transmitter sketch:
```C++
#include "TinyRF_TX.h"

void setup(){
	// transmitter default pin is pin #2. You can change it by editing Settings.h
	setupTransmitter();
}

void loop(){

	const char* msg = "Hello from far away!";
	send((byte*)msg, strlen(msg));

	//make sure there's at least a MIN_TX_INTERVAL delay between transmissions
	//otherwise the receiver's behavior will be undefined
	delay(MIN_TX_INTERVAL);

	// alternatively you can provide a third argument to send a message multiple times
	// this is for reliability in case some messages get lost in the way
	// if you have error checking and sequence numbering enabled the getReceivedData() function 
	// will return TRF_ERR_DUPLICATE_MSG when receiving a duplicate, making it easy to ignore duplicates
	// it is socially more responsible to use fewer repetition to minimize your usage of the bandwidth
	// when sending multiple messages make sure you call getReceivedData() frequently in the receiver 
	// the receiver has a 256 byte FIFO buffer, if you send too many messages and/or if they are
	// too long previous messages will get overwritten in the buffer
	sendMulti((byte*)msg, strlen(msg), 10);

	//note that even tho we are sending the same message with sendMulti, the first time the 
	//receiver doesn't detect it as a duplicate, because the sequence number is different
	//but the other 9 messages sent with sendMulti will be detected as duplicate

	delay(MIN_TX_INTERVAL);
	send((byte*)"-----", 5);

	delay(1000);
	
}
```

### Receiver sketch:
```C++
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

	// the receiver has a 256 byte FIFO buffer
	// if your loop duration is longer than the interval you send your messages then messages might
	// get accumulated in the buffer. in order to empty the buffer call getReceivedData() in a loop
	// this is specially the case when you use sendMulti()
	// note that DUPLICATE MESSAGES DO NOT GET REMOVED FROM THE BUFFER UNTIL YOU CALL getReceivedData() 
	// as many times as necessary to read them all 

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

	// this will only work if error checking and sequence numbering are enabled
	if(err == TRF_ERR_DUPLICATE_MSG){
		Serial.println("Duplicate message.");
		return;
	}

	// if sequence numbering is enabled and you use the sendMulti() function for sending a message
	// multiple times then getReceivedData() will return TRF_ERR_SUCCESS only once!
	// this means if you do sendMulti() you dont't have to check for duplicates in your code
	// you only need to read messages that return TRF_ERR_SUCCESS
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
```

## How to reduce memory usage?
If you're running out of memory here are a few TinyRF-specific and general hints:
* Disable sequence numbering: If you don't need the sequence numbering feature you can disable it by uncommenting `#define TRF_SEQ_DISABLED` in `Settings.h`. Note that disabling sequence numbering will also disable TinyRF's ability to detect duplicate messages and the `TRF_ERR_DUPLICATE_MSG` return code.
* Use checksum for error detection: Checksum detects less errors compared to CRC (the default error checking) but also occupies less memory. To use checksum instead of CRC uncomment `#define TRF_ERROR_CHECKING_CHECKSUM` in `Settings.h` and comment out the CRC define.
* Disable error checking altogether: If you don't need the error checking you can completely disable it by uncommenting `#define TRF_ERROR_CHECKING_NONE` in `Settings.h`. You should also comment checksum and CRC.
* Use `PROGMEM` for storing strings.
* Check out [Efficient C Coding for AVR](https://teslabs.com/openplayer/docs/docs/prognotes/efficient_c_coding_avr.pdf). Specially the section about "Variables and Data
Types".
* Use Atmel Studio and write in pure C instead of using Arduino. There is some overhead when using even the `setup()` and `loop()` function so if you need every last byte you should forget Arduino libraries and just use purce C.

## How to use without Arduino?
In order to make the library easy to use for Arduino users I have written the library with Arduino functions such as `digitalWrite()` and `delayMicroseconds()`. The optimizer automatically takes care of them and they don't add an overhead (tested with ATtiny13 + MicroCore). If you want to use it in a pure C AVR project you'll have to replace the functions yourself.

## How to change settings:
Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. To find out which settings are available and what they do take a look at `Settings.h`.

### Feel free to create an issue if you encountered any bugs or problems