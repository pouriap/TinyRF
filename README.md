# TinyRF

A small Arduino library for generic 315MHz / 433MHz RF modules.

The transmitter code is small in size making it suitable for microcontrollers with limited memory. Namely the ATtiny13, or other ATtiny chips. But Arduino and other AVR MCUs are also supported.

![433MHz / 315MHz cheap ebay RF modules](https://repository-images.githubusercontent.com/293609741/4b910480-f297-11ea-96e6-fd41628b4086)

## Features
* Built-in error checking
* Built-in sequence numbering
* Data transmission speeds up to 1000bps (2000bps with calibrated oscillator)
* Ability to disable features in order to preserve memory space
* Small memory footprint: 176 bytes Flash, 1 byte RAM with all features enabled

**Transmitter MCU support:** ATtiny13 or any other microcontroller you can program with Arduino IDE.

**Receiver MCU support:** As of version 2.0 of the library even an ATtiny13 can be used as the receiver! However the proper header files have to be included. Please read the [library setup](#library-setup) section for more info.

**Arduino IDE support:** Arduino IDE 1.6.0 and higher.

## How to install the library
- Download the [latest release](https://github.com/pouriap/TinyRF/releases/latest) 
- Open Arduino IDE and go to Sketch -> Include Library -> Add .ZIP Library...
- Alternativey you can copy the contents of the zip file in your Arduino "libraries" folder. 
- Restart Arduino IDE.

## Usage notes
* This library has multiple configurations. Please avoid including it using the "include library" feature and use the examples as the starting point. The libaray changes behavior based on what header files you include.
* Tips and information about using the libaray are provided in the comments of example files. Make sure to read them.
* The internal clock(s) of the ATtiny13 can be inaccurate. Specially the 4.8MHz oscillator because by default only the calibration data for the 9.6MHz oscillator is copied. I highly recommend that you [calibrate your chip](https://github.com/MCUdude/MicroCore#internal-oscillator-calibration) to get more accurate timings. The library might not even work depending on how inaccurate your chip is.
* Make sure you call `getReceivedData()` as frequently as possible in your receiver sketch loop. Specially if you are using the **Tiny13** version of the library since it does not have a buffer.
* In the **Standard** version of the library you can technically send messages as long as 250 Bytes long but that is not recommended. The longer your messages are the more susceptible to noise they become. Also the error checking byte will detect less and less errors the longer your message is.
* Check out `Settings.h` to find out which settings are available and what they do.
* Don't forget proper powering! A 0.1uF decoupling cap for the MCU is **mandatory**. I personally recommend an additional 22uF across the MCU and at least 100uF across the transmitter and receiver modules. Also use a nice and stable power source. This will minimize errors and headaches.

## Library setup
Depending on what MCU you are using as the receiver you have to include different files in your sketch and take certain considerations into account:
* **Standard version (Arduino Uno and similar):** Since these microcontrollers have enough RAM and FLASH memory the standard library files can be used with them. The standard library has features like a large circular buffer, automatic sequence numbering, better error chekcing and certain helper functions. 
  
  Include `TinyRF_TX.h` and `TinyRF_RX.h` to use this version of the library. Refer to the "Standard" subfolder of the examples for more info.
* **Tiny85 version (ATtiny85):** Currently there is a specific version for ATtiny85 that has all the features of the standard version only with a smaller 16-byte buffer. This version is intended to be deprecated in the future.  
  
  Include `TinyRF_85_TX.h` and `TinyRF_85_RX.h` to use this version of the library.Refer to the "Tiny85" subfolder of the examples for more info. 
* **Tiny13 version (ATtiny13/25/45/85 + ATmega328):** As of version 2.0 of the library even ATtiny13 can be used as the receiver. This version of the library is completely different from the standard version with many limitations:
  1. Only two bytes of data can be sent in each packet (a `uin16_t` value)
  2. There is no buffer in the receiver and if data is not read in time it will be lost
  3. Fancy stuff such as sequence numbering and sendMulti() function are removed and if you need such functionaltiy you should implement them yourself.
  
  This version also supports Arduino Uno (ATmega328P) as the receiver so if for any reason you are low on resources you can use this version of the library with the UNO as receiver.  
  
  Include `TinyRF_13_TX.h` and `TinyRF_13_RX.h` to use this version of the library.Refer to the "Tiny13" subfolder of the examples for more info.

## How to reduce memory usage?
If you're running out of memory here are a few TinyRF-specific and general hints:
* Use checksum for error detection: Checksum detects less errors compared to CRC (the default error checking) but also occupies less memory. To use checksum instead of CRC uncomment `#define TRF_ERROR_CHECKING_CHECKSUM` in `Settings.h`.
* Disable sequence numbering: If you don't need the sequence numbering feature you can disable it by uncommenting `#define TRF_SEQ_DISABLED` in `Settings.h`. Note that disabling sequence numbering will also disable TinyRF's ability to detect duplicate messages sent by `sendMulti()`.
* Disable error checking altogether: If you don't need the error checking you can completely disable it by uncommenting `#define TRF_ERROR_CHECKING_NONE` in `Settings.h`.
* Use `PROGMEM` for storing strings.
* Check out [Efficient C Coding for AVR](https://teslabs.com/openplayer/docs/docs/prognotes/efficient_c_coding_avr.pdf). Specially the section about "Variables and Data
Types".
* Use Atmel Studio and write in pure C instead of using Arduino. There is some overhead when using even the `setup()` and `loop()` function so if you need every last byte you should forget Arduino libraries and just use purce C.

## How to use without Arduino?
In order to make the library easy to use for Arduino users I have written the library with Arduino functions such as `digitalWrite()` and `delayMicroseconds()`. The optimizer automatically takes care of them and they don't add an overhead (tested with ATtiny13 + MicroCore). If you want to use it in a pure C AVR project you'll have to replace the functions yourself.

## How to change settings:
Transmitter pin number and other settings are defined in `Settings.h` instead of being set programatically in order to save program space. To find out which settings are available and what they do take a look at `Settings.h`.

### Feel free to create an issue if you encountered any bugs or problems