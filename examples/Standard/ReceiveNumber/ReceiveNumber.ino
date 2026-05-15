#include "TinyRF_RX.h"

// the pin which will be connected to receiver module
// this pin has to support EXTERNAL interrupts
uint8_t rxPin = 2;

void setup(){
  Serial.begin(115200);
  //make sure you call this in your setup
  setupReceiver(rxPin);
}

void loop(){

  const uint8_t bufSize = 7;
  byte buf[bufSize];
  uint8_t numRcvdBytes = 0;

  uint8_t err = getReceivedData(buf, bufSize, numRcvdBytes);

  if(err == TRF_ERR_NO_DATA){
    return;
  }

  if(err == TRF_ERR_BUFFER_OVERFLOW){
    Serial.println("Buffer too small for received data!");
    return;
  }

  if(err == TRF_ERR_CORRUPTED){
    Serial.println("Received corrupted data.");
    return;
  }

  if(err == TRF_ERR_SUCCESS){

    byte byteNum;
    int intNum;
    long longNum;

    byteNum = buf[0];

    //now we do the opposite of what we did in the transmitter sketch
    //pay attention to the '|=' operator
    //only the first byte assignment uses the '=' operator
    intNum = (int) buf[1];
    intNum |= (int) buf[2] << 8;

    longNum = (long) buf[3];
    longNum |= (long) buf[4] << 8;
    longNum |= (long) buf[5] << 16;
    longNum |= (long) buf[6] << 24;

    Serial.print("byte number received: ");
    Serial.println(byteNum);

    Serial.print("int number received: ");
    Serial.println(intNum);

    Serial.print("long number received: ");
    Serial.println(longNum);

  }
  
}