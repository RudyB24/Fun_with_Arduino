## Kitchen LED with PIR sensor

This Arduino sketch controls the brightness of an LED strip via a FET module that is connected to a PWM output.

A PIR sensor is used to detect presence, after which the LEDs fade on. When the PIR sensor doesn't detect presence anymore, after a specified amount of time the LEDs fade off.

There's a 'sensor' state, a 'continuous on' state and an off state. The encoder pushbutton is used to switch between the states using short or long presses:
- state1: fade to brightness[0], sensor active, short press >> state3, long press >> state3
- state1: if detector sees nothing, after STAY_ON_TIME the LEDs turn off
- state2: brightness[1], sensor not active so LEDs stay on, short press >> state1, long press >> state3
- state3: transition state to write brightness values to EEPROM and go to state4
- state4: LEDs off. Short press >> state1, long press >> state2

The brightness in both states can be adjusted by rotating the encoder. They are stored in EEPROM in state3.

--  0 --
