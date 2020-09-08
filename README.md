# TinyRF

A library to use cheap 315/433Mhz RF modules with ATtiny13 as transmitter.

## Features
* Built-in error checking
* Built-in sequence numbering
* Data transmission speeds up to 1100bps (2500bps if using Arduino as transmitter)
* Up to 255 byte message length
* Ability to disable different features in order to preserve memory space
* Relatively small memory footprint (~300bytes FLASH, 5bytes RAM with all features enabled)

Transmitter MCU support: ATtiny13 and anything else.

Receiver MCU support: Currently a more capable board like an Arduino is needed for receiver because the receiver part needs more resources.

Arduino IDE support: Arduino IDE 1.6.0 and higher.

## How to use
Copy the contents of the repository in your Arduino libraries folder in a subfolder called `TinyRF`. Alternatively you can just copy/pase all `.h` and `.cpp` files in your sketch folder.

### Transmitter sketch:
```
#include "TinyRF_TX.h"

void setup(){
	setupTransmitter();
}

void loop(){
	const char* msg = "Hello from far away!";
	send((byte*)msg, strlen(msg));
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

## Notes:
Transmitter default pin is pin #2. For changing that edit the `Settings.h` file. Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. To find out which settings are available and what they do take a look at `Settings.h`. 
