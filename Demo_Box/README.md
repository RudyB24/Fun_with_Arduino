The Demo Box contains:
- 2 toggle switches
- 2 pushbuttons
- 1 potentiometer
- 1 digital encoder with pushbutton
- 6 LEDs
- 1 servo

// The pin numbers are for the demo box with the flatcable in the aluminium case
#define SERVO_PIN  12 
#define ENCA_PIN   14 // A0 Encoder A or Data
#define ENCB_PIN   15 // A1 Encoder B or Clock
#define POTM_PIN   17 // A3 Potentiometer Analog In
byte swpin[]     = {8,9};      // SW0, SW1 input pins
byte pbpin[]     = {10,11,16}; // PB0, PB1, encoder input pins
byte digledpin[] = {2,4,7,13}; // digital LED outputs pins, 13 is PCB LED
byte pwmledpin[] = {3,5,6};    // pwm LED output pins NOTE: marked ~ , don't use pins 9,10 with servo.h

With almost always every code idea we want to try some physical inputs and visible outputs are required. The purpose of this 'Demo Box' is to have these readily available for quick and easy use.

The Demo_Box.ino file contains examples of code snippets that are common in many applications.

**BEWARE: If servo.h is included, then analogWrite() does not work for pins 9,10 !**

**Read toggle switch inputs.**  
The pin numbers of the switch inputs are configured in the sw_pin[i] array. They are read every loop() cycle. The values are stored in the sw[i] array. The toggle switches are debounced, they only schange state if their state is stable longer than DEBOUNCE ms. A state change of the switches, from 0 -> 1 or from 1 -> 0, is stored in the sw_change[i] array for the duration of one cycle. This can be used to do something only once after the switch state has changed.

**Read pushbuttons inputs**  
The pin numbers of the pushbutton inputs are configured in the pb_pin[i] array. The values are stored in the pb[i] array. The pushbuttons are read in a function that returns:
- 2 if pressed longer than LONG_PRESS ms
- 1 if pressed longer than SHORT_PRESS ms
- 0 if pressed shorter than SHORT_PRESS ms, which automatically functions as debounce

**Read digital encoder**  
The digital encoder is read in a function that returns:
- 10 if rotation was faster than ENC_SLOW_FAST ms
- 1 if rotation was slower than ENC_SLOW_FAST ms
- 0 if pulses were shorter than ENC_DEBOUNCE ms


