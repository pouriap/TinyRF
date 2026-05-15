#include <TinyRF_13_TX.h>

/** Important notes for the transmitter:
* - The default pin for the transmitter is pin#0.
* - Any pin can be used for the transmitter but you need to change it in the library header file (Arduino/libraries/TinyRF-lite/src/TRFLite_TX.h)
* - If you want to free up program space you can go to the settings file (Arduino/libraries/TinyRF-lite/src/settings.h) and uncomment the lines as instructed there.
*/

void setup()
{
    setupTransmitter();
}

// This should create a remote blink on the receiver
void loop()
{
    // send 3 times to make sure it reaches the receiver
    for(uint8_t i=0; i<3; i++)
    {
        send(0x1234);
        delay(5);
    }
    delay(1000);

    for(uint8_t i=0; i<3; i++)
    {
        send(0x4321);
        delay(5);
    }
    delay(1000);
}