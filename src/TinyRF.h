#ifndef TINYRF_H
#define TINYRF_H

#include "Arduino.h"

//Do not increase this above 255, because all data types related to messages are uint8_t so 
//increasing this will break the program. 
//For example 'len' arugment of send() function is uint8_t so you can't 
//send anything longer than 255 bytes even if you increase this
const uint8_t MAX_MSG_LEN = 255;

//where should we do end of transmission check?
//best place is in TX but if you have limited memory you can do it in RX but that will increase
//error rate. alternatively you can just disable it using EOT_NONE.
#define EOT_IN_RX
//#define EOT_NONE


#define fast

#ifdef slow
const unsigned int START_PULSE_DURATION = 8000;
const unsigned int ONE_PULSE_DURATION = 5000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
#endif
 
#ifdef good
const unsigned int START_PULSE_DURATION = 6000;
const unsigned int ONE_PULSE_DURATION = 4000;
const unsigned int ZERO_PULSE_DURATION = 3000;
const unsigned int HIGH_PERIOD_DURATION = 2000;
#endif

#ifdef fast
const unsigned int START_PULSE_DURATION = 3000;
const unsigned int ONE_PULSE_DURATION = 2000;
const unsigned int ZERO_PULSE_DURATION = 1500;
const unsigned int HIGH_PERIOD_DURATION = 1000;
#endif

#ifdef superfast
const unsigned int START_PULSE_DURATION = 2000;
const unsigned int ONE_PULSE_DURATION = 1000;
const unsigned int ZERO_PULSE_DURATION = 800;
const unsigned int HIGH_PERIOD_DURATION = 500;
#endif

#ifdef lightning
const unsigned int START_PULSE_DURATION = 2000;
const unsigned int ONE_PULSE_DURATION = 400;
const unsigned int ZERO_PULSE_DURATION = 300;
const unsigned int HIGH_PERIOD_DURATION = 200;
#endif

//Function declarations
byte checksum8(byte data[], uint8_t len);
byte crc8(byte data[], uint8_t len);


#endif  /* TINYRF_H */ 
