#ifndef TRFLITE_RX_CPP_
#define TRFLITE_RX_CPP_

#include "TRFLite_RX.h"

namespace trflite
{
	volatile MICROSIZE useconds = 0;
	volatile MICROSIZE lastChangeTime = 0UL;
	DataField global;
}

ISR(TIMER_VECTOR)
{
	using namespace trflite;
	useconds += 25;
}

ISR(INT0_vect)
{
	using namespace trflite;

	#ifdef MCU_TINY
	MICROSIZE now = useconds;
	#else
	MICROSIZE now = micros();
	#endif

	MICROSIZE dur = now - lastChangeTime;

	// ignore noise
	if(dur < ZERO_PERIOD_DETECT) 
	{
		lastChangeTime = now;
		return;
	}

	// START bit received
	if(dur > START_PERIOD_DETECT)
	{
		global.bitIndex = 0;
		global.rxBuf = 0L;
	}	

	// ONE bit received
	else if(dur > ONE_PERIOD_DETECT)
	{
		bitWrite(global.rxBuf, global.bitIndex, 1);
		global.bitIndex++;
	}

	// ZERO bit received
	else if(dur > ZERO_PERIOD_DETECT)
	{
		global.bitIndex++;
	}

	lastChangeTime = now;
}

void setupReceiver()
{
	cli();

	#ifdef MCU_TINY
	GIMSK |= (1 << INT0); 		// enable external interrupt
	MCUCR |= (1 << ISC01); 		// interrupt on falling change
	GIFR |= (1 << INTF0); 		// enable INT0 interrupt routine

    TCCR0A = 0;
    TCCR0B = 0;
    TCNT0 = 0;
    TIMSK0 = 0;

    TCCR0A |= (1 << WGM01);   	// CTC mode
	TCCR0B |= PRESCALER;
    TIMSK0 |= (1 << OCIE0B); 	// enable timer interrupt on OC0B
    OCR0A = OCR_VAL;			// interrupt every 25us
	#else
	EICRA |=  (1 << ISC01);		// interrupt on falling change
	EIMSK |= (1 << INT0);		// enable external interrupt
	#endif

	sei();
}

RXMsg getReceivedData()
{
	using namespace trflite;

	RXMsg msg = {0, false};

	if(global.bitIndex == DATA_SIZE)
	{
		#ifdef MCU_TINY
		GIMSK &= ~(1 << INT0);
		#else
		asm volatile("cli"); // Disable interrupts
		#endif

		uint8_t rcvdCRC = global.rxBuf & 0xFF;
		msg.data = (global.rxBuf >> 8) & 0xFFFF;

		#ifdef MCU_TINY
		GIMSK |= (1 << INT0);
		#else
		asm volatile("sei"); // Enable interrupts
		#endif

		uint8_t crc = CRC_FUNC(msg.data);

		if(rcvdCRC == crc) msg.dataValid = true;
	}

	return msg;
}

#ifndef DISABLE_MICROS
uint32_t microSeconds()
{
	using namespace trflite;
	return useconds;
}
#endif

#endif /* TRFLITE_RX_CPP_ */