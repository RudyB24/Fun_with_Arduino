The Demo Box contains:
- 2 toggle switches
- 2 pushbuttons
- 1 potentiometer
- 1 digital encoder with pushbutton
- 6 LEDs
- 1 servo

The purpose is to be able to easily try out all kinds of code ideas we may have ... almost always some physical inputs and visible outputs are required, the Demo Box delivers these quick and easy, readily available.

The Demo_Box.ino file contains examples of code snippets that are common in many applications.

**BEWARE:   
If servo.h is included, then analogWrite() does not work anymore for pins 9,10 !**

**Read toggle switch inputs.**  
They are read every loop() cycle and stored in the sw[i] array. Also a state change of the switches, from 0 -> 1 or from 1 -> 0, is stored in the sw_change[i] array for the duration of one cycle. This can be used to do something only once, if the switch state has changed.  
The pin numbers of the switch inputs are configured in the sw_pin[] array.

**Debounce an input.**  
The toggle switches are debounced, they only schange state after a DEBOUNCE time in ms.

**Read pushbuttons inputs**  
The pushbuttons are read in a function that returns:
- 2 if pressed longer than LONG_PRESS
- 1 if pressed longer than SHORT_PRESS
- 0 if pressed shorter than SHORT_PRESS, which automatically functions as debounce  
The pin numbers of the switch inputs are configured in the sw_pin[] array.
