#ifndef TRFLITE_H_
#define TRFLITE_H_

#include "Arduino.h"
#include "TRFLite_Settings.h"

#define DUR_ZERO 500
#define DUR_ONE 1000
#define DUR_SPACE 500
#define DUR_START 1500
#define DUR_ERR 100

#define ZERO_PERIOD_DETECT (DUR_ZERO + DUR_SPACE - DUR_ERR)
#define ONE_PERIOD_DETECT (DUR_ONE + DUR_SPACE - DUR_ERR)
#define START_PERIOD_DETECT (DUR_START + DUR_SPACE - DUR_ERR)

#define DATA_SIZE 24

#ifndef USE_CHECKSUM
#define CRC_FUNC crc8_fast
#else
#define CRC_FUNC checksum8
#endif

uint8_t checksum8(uint16_t data);
uint8_t crc8_fast(uint16_t data);


#endif    // TRFLITE_H_
