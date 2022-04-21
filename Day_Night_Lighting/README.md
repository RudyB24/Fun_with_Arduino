Ruud Boer - April 2022

This Arduino sketch controls 4 PWM outputs to which 4 LED strips, or an RGB plus a separate W strip, or one RGBW strip, can be connected via 4 FET modules.

Five lighting states each have their own RGBW values:
0 Day
1 Evening
2 Night
3 Morning
4 Work light

4 inputs (switches, not pushbutton, active when GND) are used to enable one of the states 0-3
A 5th input switches a sequencer that steps from one state to the next via a gradual fade
If no input is present the state is 4: Work Light

-- 0 --
