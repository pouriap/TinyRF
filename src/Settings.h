#ifndef TINYRF_SETTINGS_H
#define TINYRF_SETTINGS_H


/**
 * The pin that is connected to the transmission module
 * We use a #define instead of settting it programatically in order to save program space on TX
**/
#define txPin 2


/**
 * What kind of error checking should be used
 * CRC detects more errors but uses ~20bytes more program space
 * Checksum detects less errors
 * Alternatively you can use ERROR_CHECKING_NONE to disable error checking
 * Note that error checking can only detect if a data was curropted during transmission. 
 * It cannot recover the original data. So you still need to send a message multiple times to 
 * make sure it is received.
**/
#define ERROR_CHECKING_CRC
//#define ERROR_CHECKING_CHECKSUM
//#define ERROR_CHECKING_NONE


/**
 * Whether sequence numbering should be disabled
 * It is enabled by default. Uncomment the below define to disable it.
 * If enabled a sequence number will be attached to every message sent and in the receiver 
 * the sequence number will be checked to determine if any messages have been lost
 * The sequence number resets at 255 so if there's more than 255 messages lost we won't find out
**/
//#define TX_NO_SEQ


/**
 * Data rate presets
 * Available presets are: BITRATE_240, BITRATE_500, BITRATE_1100, BITRATE_2500
 * According to the datasheet uncalibrated ATtiny13 has 10% frequency error.
 * We also need at least 30us of error margin because we use delayMicroseconds() which isn't super accurate.
 * So if our 0 pulse period is 500 then our error margin is 500-50-30 to 500+50+30
 * In my experience BITRATE_1100 was a good option with ATtiny13 as TX
 * If you are using an Arduino as TX you can go up to BITRATE_2500 speed
 * If you need custom speeds you can define one for yourself in the header file "TinyRF.h"
 *
 * Preabmle:
 * Preabmle to send before each transmission to get the receiver tuned.
 * Increase preabmle if you decrease pulse periods.
 * In my experiments I needed ~50ms of preamble (the faster the datarate the more preabmle needed).
 * However Internet suggests much shorter times.
**/
#define BITRATE_1100


/**
 * If you are using an uncalibrated ATtiny as transmitter leave this define as it is
 * If your ATtiny is calibrated or if you're using a crystal oscillator or if you're using
 * an Arduino as transmitter comment out this line
**/
#define TX_ATTINY_UNCALIBRATED


/**
 * We don't have a specific pulse that signals the end of a transmission. We rely on noise for that.
 * When noise is received in the receiver the transmission is considered over.
 * But in the rare even that you are in a noiseless environment this could mean that the receiver 
 * will keep waiting for the next byte of the transmission. There are two ways we can fix that
 * 1- Create noise in the TX: i.e. send a bunch of meaningless pulses 
 * 2- Detect end of transmission in RX: i.e. when no data has been received for a while considere the transmission finished
 * The default is EOT_IN_RX because we want to minimize the transmitter code size
 * You can uncomment EOT_IN_TX if you want EOT to be done in transmitter
 * Alternatively you can uncomment EOT_NONE if you think you don't need this
**/
//#define EOT_IN_TX
//#define EOT_NONE


#endif	/* TINYRF_SETTINGS_H */