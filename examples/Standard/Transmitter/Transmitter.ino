#include "TinyRF_TX.h"

void setup(){
  // transmitter default pin is pin #2. You can change it by editing Settings.h
  setupTransmitter();
}

void loop(){

  const char* msg = "Hello from far away!";

  //send function accepts an array of bytes as first argument
  //second argument is the length of the array
  send((byte*)msg, strlen(msg));

  //make sure there's at least a TX_DELAY_MICROS delay between transmissions
  //otherwise the receiver's behavior will be undefined
  delayMicroseconds(TX_DELAY_MICROS);
  
}