# TinyRF

A library to use cheap 315/433Mhz RF modules with ATtiny13 as transmitter.

## Features
* Different data transmission speeds up to 1100bps (2500bps if using Arduino as transmitter)
* Up to 255 byte message length
* CRC/checksum error checking
* Customizable

Transmitter MCU support: ATtiny13 and anything else.

Receiver MCU support: Currently Arduino and other higher-resource MCUs are needed as receiver because it needs more resources.

## How to use
Copy the repository in your libraries folder.

### Transmitter sketch:
	#include "TinyRF_TX.h"
	void setup(){
		setupTransmitter();
	}
	void loop(){
		const char* msg = "Hello!";
		send((byte*)msg, strlen(msg)+1); //include null character
	}

### Receiver sketch:
	#include "TinyRF_RX.h"
	int rxPin = 2;
	void setup(){
		Serial.begin(9600);
		setupReceiver(rxPin);
	}
	void loop(){
		char buf[10];
		int err = getReceivedData(buf);
		if(err == TINYRF_ERR_SUCCESS){
			Serial.print("Received: ");
			Serial.println(buf);
		}
	}

## Notes:
Transmitter default pin is pin #2. For changing that edit the `Settings.h` file. Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. 

<br>
(This readme will be updated)