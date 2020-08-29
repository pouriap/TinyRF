#include "Arduino.h"
#include "ATtiny13_RF.h"

const unsigned int START_PULSE_DURATION = 1500;
const unsigned int START_TRIGGER = 1400;
const unsigned int ONE_PULSE_DURATION = 1000;
const unsigned int ONE_TRIGGER = 900;
const unsigned int ZERO_PULSE_DURATION = 500;
const unsigned int ZERO_TRIGGER = 400;
const unsigned int HIGH_PERIOD_DURATION = 100;

const int FRAME_PULSES = 8;	//8 pulses of data
unsigned int pulses[FRAME_PULSES];
volatile uint8_t receivedData = 0x00;
unsigned long duration = 0;


/**
 * data: array of bytes
 * len: array size / number of bytes
**/
uint8_t checksum8(uint8_t data[], uint8_t len){
	uint16_t sum = 0;
	// Compute the sum.  Let overflows accumulate in upper 8 bits.
	for(uint8_t i=0; i<len; i++){
		//Serial.print("adding: ");Serial.print(~data[i]);Serial.print(" 0x");Serial.println(~data[i], HEX);
		sum += (~data[i] & 0xFF);
		//Serial.print("sum is: ");Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);
	}
	//Serial.print("sum after IF: "); Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);
	// Now fold the overflows into the lower 8 bits.
	sum = (sum & 0xFF) + (sum >> 8);
	//Serial.print("sum after carry: ");Serial.print(sum);Serial.print(" 0x");Serial.println(sum, HEX);

	// Return the 1s complement sum in finalsum
	return (uint8_t) ~sum;
}

void enableReceive(uint8_t pin){
	attachInterrupt(digitalPinToInterrupt(pin), interrupt_routine, FALLING);
}

bool isPulseTriggered(unsigned int pulse, unsigned int trigger){
	if( (pulse < (trigger+20)) && (pulse > (trigger-20)) ){
		return true;
	}
	return false;
}

void process_received(){
	receivedData = 0x00;
	for(int i=0; i<FRAME_PULSES; i++){
		//if pulse is greater than 200us then it will not be here
		//if it's greater than 125 it's a high pulse
		//else it a low pulse
		//if(isPulseTriggered(pulses[i], ONE_PULSE_DURATION)){
		if(pulses[i] > ONE_TRIGGER){
			receivedData |= (1<<i);
		}
		//else if(!isPulseTriggered(pulses[i], ZERO_PULSE_DURATION)){
		else if(pulses[i] < ZERO_TRIGGER){
			//curropted
			receivedData = 0;
			return;
		}
	}
	Serial.print("received: ");Serial.println(receivedData);
}

void interrupt_routine(){

	static unsigned long lastTime = 0;
	static unsigned int pulse_count = 0;
	static bool startReceived = false;

	unsigned long time = micros();
	unsigned int pulseDuration = time - lastTime;
	
	//start of transmission
	//if(isPulseTriggered(pulseDuration, START_PULSE_DURATION)){
	if(pulseDuration > START_TRIGGER){
		duration = pulseDuration;
		startReceived = true;
		pulse_count = 0;
	}
	else if(startReceived){
		pulses[pulse_count] = pulseDuration;
		pulse_count++;
	}

	if(pulse_count == FRAME_PULSES){
		process_received();
		startReceived = false;
		pulse_count = 0;
	}

	lastTime = time;

}

void send(uint8_t data, uint8_t pin){

	digitalWrite(pin, LOW);
	delayMicroseconds(START_PULSE_DURATION - HIGH_PERIOD_DURATION);
	digitalWrite(pin, HIGH);
	delayMicroseconds(HIGH_PERIOD_DURATION-4);	//-4 because digitalWrite takes 3.6us
	for(int i=0; i<8; i++){
		//if 1
		if(data & (1<<i)){
			digitalWrite(pin, LOW);
			delayMicroseconds(ONE_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION-4);
		}
		else{
			digitalWrite(pin, LOW);
			delayMicroseconds(ZERO_PULSE_DURATION - HIGH_PERIOD_DURATION);
			digitalWrite(pin, HIGH);
			delayMicroseconds(HIGH_PERIOD_DURATION-4);
		}
		digitalWrite(pin, LOW);
	}
}

uint8_t getReceivedData(){
	return receivedData;
}

unsigned long getDuration(){
	return duration;
}