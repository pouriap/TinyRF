#include <TinyRF_13_RX.h>

/** Important note for the receiver:
* - Supported CPU frequencies are 9.6Mhz, 4.8Mhz and 8Mhz.
* - If you are using the 4.8Mhz on ATtiny13 you should first calibrate the MCU.
* - The receiver uses a custom function for getting time called microSeconds(). It is advised that if you need time in your receiver code,
* to use the microSeconds() function that comes with the library instead of the default micros() function.
* - The default RX pin is pin#1 on ATtiny13 and pin#2 on Attiny25/45/85.
* - The receiver uses the external interrup so it can't be used by the user or any other library.
* - The receiver also uses the TIMER0_COMPB interrupt vector so it's also unavailable for other purposes.
* - If you want to free up program space you can go to the settings file (Arduino/libraries/TinyRF-lite/src/settings.h) and uncomment the lines as instructed there.
* - Hardware notes:
* - Always add a 100nF capacitor to ther power pin of the MCU
* - Highly recommended to add a 100uF capacitor to the transmitter and receiver modules
* - Since the receiver module outputs a lot of noise I recommend adding a ~1.5KHz(10KOhm+ 10nF) RC filter from the receiver pin into the MCU to 
* filter out some of the noise, specially if the receiver is on the 4.8Mhz frequency because the interrupts will take too much of its time.
*/

void setup()
{
    setupReceiver();
    pinMode(LED_BUILTIN, OUTPUT);
}

// Connect an LED to the built-in LED pin (usually pin 2 on ATtiny) and it sould blink every 1 second with the example transmitter code
void loop()
{
    RXMsg msg = getReceivedData();
    if(msg.dataValid)
    {
        if(msg.data == 0x1234) digitalWrite(LED_BUILTIN, HIGH);
        if(msg.data == 0x4321) digitalWrite(LED_BUILTIN, LOW);
    }
}