#ifndef TRFLITE_RX_H_
#define TRFLITE_RX_H_

#include "TRFLite.h"

//todo: does tiny25/45/85 need cutoms micros()?
#if defined(__AVR_ATtiny13__)
	#define MCU_TINY
	#define RX_PIN PB1
	#define TIMER_VECTOR TIM0_COMPB_vect
#elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	#define MCU_TINY
	#define RX_PIN PB2
	#define TIMER_VECTOR TIMER0_COMPB_vect
#elif defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__)
	#define MCU_328
	#define RX_PIN PD2
#else
	#error "Unsupported MCU"
#endif

#ifdef MCU_TINY
	#if (F_CPU == 9600000L)
	#define OCR_VAL 29
	#define PRESCALER (1 << CS01)	// prescalar 8
	#elif (F_CPU == 4800000L)
	#define OCR_VAL 14
	#define PRESCALER (1 << CS01)	// prescalar 8
	#elif (F_CPU == 8000000L)
	#define OCR_VAL 24
	#define PRESCALER (1 << CS01)	// prescalar 8
	#else
	#error "Unsupported CPU frequency!"
	#endif
#endif

#ifndef DISABLE_MICROS
#define MICROSIZE uint32_t
#else
#define MICROSIZE uint16_t
#endif

typedef struct
{
	uint16_t data;
    bool dataValid;
} RXMsg;

typedef struct
{
	uint8_t bitIndex;
	uint32_t rxBuf: DATA_SIZE;
} DataField;

void setupReceiver();
RXMsg getReceivedData();

#ifndef DISABLE_MICROS
uint32_t microSeconds();
#endif

#endif    // TRFLITE_RX_H_
