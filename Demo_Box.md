The Demo Box contains:
- 2 toggle switches
- 2 pushbuttons
- 1 potentiometer
- 1 digital encoder with pushbutton
- 6 LEDs
- 1 servo

The purpose is to be able to easily try out all kinds of code ideas we may have ... almost always some physical inputs and visible outputs are required, the Demo Box delivers these quick and easy, readily available.

**BEWARE: If servo.h is included, then analogWrite() does not work anymore for pins 9,10 !**

The Demo_Box.ino file contains examples of code snippets that are common in many applications. The choice that was made is to read all theh toggle switch- and pushbutton inputs every loop() cycle and store their values in the sw[] and pb[] arrays.
- Debounce an input. Used here with the toggle switches

