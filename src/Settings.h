#ifndef TRF_SETTINGS_H
#define TRF_SETTINGS_H


/**
 * The pin that is connected to the transmission module
 * We use a #define instead of settting it programatically in order to save program space on TX
**/
#define TRF_TX_PIN 2


/**
 * What kind of error checking should be used
 * CRC detects more errors but uses ~20bytes more program space
 * Checksum detects less errors
 * Alternatively you can use TRF_ERROR_CHECKING_NONE to disable error checking
 * Note that error checking can only detect if a data was corrupted during transmission. 
 * It cannot recover the original data. So you still need to send a message multiple times to 
 * make sure it is received.
**/
#define TRF_ERROR_CHECKING_CRC
//#define TRF_ERROR_CHECKING_CHECKSUM
//#define TRF_ERROR_CHECKING_NONE


/**
 * Whether sequence numbering should be disabled
 * It is enabled by default. Uncomment the below define to disable it.
 * If enabled a sequence number will be attached to every message sent and in the receiver 
 * the sequence number will be checked to determine if any messages have been lost
 * The sequence number resets at 255 so if there's more than 255 messages lost we won't find out
**/
//#define TRF_SEQ_DISABLED


/**
 * Data rate presets
 * Available presets are: TRF_BITRATE_240, TRF_BITRATE_500, TRF_BITRATE_1100, TRF_BITRATE_2500
 * According to the datasheet uncalibrated ATtiny13 has 10% frequency error.
 * We also need at least 30us of error margin because we use delayMicroseconds() which isn't super accurate.
 * So if our 0 pulse period is 500 then our error margin is 500-50-30 to 500+50+30
 * In my experience TRF_BITRATE_1100 was a good option with ATtiny13 as TX
 * If you are using an Arduino as TX you can go up to TRF_BITRATE_2500 speed
 * If you need custom speeds you can define one for yourself in the header file "TinyRF.h"
 *
 * Preabmle:
 * Preabmle to send before each transmission to get the receiver tuned.
 * Increase preabmle if you decrease pulse periods.
 * In my experiments I needed ~50ms of preamble (the faster the datarate the more preabmle needed).
 * However Internet suggests much shorter times.
**/
#define TRF_BITRATE_1100


/**
 * If you are using an uncalibrated internal oscillator for transmitter choose TRF_TX_UNCALIBRATED
 * If you are using a calibrated internal oscillator for transmitter choose TRF_TX_CALIBRATED
 * If you are using an Arduino or any other MCU with a crystal oscillator choose TRF_TX_CRYSTAL
**/
#define TRF_TX_UNCALIBRATED
//#define TRF_TX_CALIBRATED
//#define TRF_TX_CRYSTAL


/**
 * [ You probably don't want to change this ]
 * We don't have a specific pulse that signals the end of a transmission. 
 * We rely on 1- noise and 2- message length for that.
 * 1) Message length is sent with each frame but it's susceptible to noise and cannot be trusted 100%.
 * 2) When noise is received in the receiver the transmission is considered over.
 * But in the rare even that you are in a noiseless environment or if you are sending messages 
 * without a delay between them (which allows for noise to cause EOT) this could mean that the 
 * receiver will keep waiting for the next byte of the transmission AND add the next preabmle to current message
 * There are two ways we can fix that
 * 1- Create noise in the TX: i.e. send a bunch of meaningless pulses 
 * 2- Detect end of transmission in RX: i.e. when no data has been received for a while consider 
 * the transmission finished.
 * Solution number 2 has a drawback: we don't use timer interrupt for checking how long there has 
 * been silence, instead we use getReceivedData() for that. So if getReceivedData() is not called 
 * frequently enough we will not be able to detect EOT (this is in case there is no noise and len is curropted)
 * So there are 3 redundant means by which we detect EOT, this means our errors will be rare
 * The default is EOT_IN_RX because we want to minimize the transmitter code size
 * You can uncomment TRF_EOT_IN_TX if you want EOT to be done in transmitter which is more reliable
 * Alternatively you can uncomment TRF_EOT_NONE if you think you don't need this
**/
//#define TRF_EOT_IN_TX
//#define TRF_EOT_NONE


#endif	/* TRF_SETTINGS_H */