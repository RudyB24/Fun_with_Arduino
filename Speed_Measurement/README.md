This Arduino sketch measures the speed of a moving object as it passes two sensors that are spaced on a given distance.

It can be used as a measurement device for model trains.

Settings can be changed via inputs:
- default:  scale HO 1/87
- A0 = LOW: scale O 1/45
- A1 = LOW: scale OO 1/76
- A2 = LOW: scale TT 1/120
- A3 = LOW: scale N 1/160
- 8 = HIGH: units km/hr
- 8 = LOW:  units MPH

The sensor distance and the sensors pin connections have to be defined in the code.

The next measurement can only be started once the train has fully passed the second sensor. The unit calculates the time this takes from the measurement it just performed and the length of the train. The train length is 200 mm by default but it can be changed via keyboard input in the Arduino IDE Serial Monitor, just type the klength in cm there.

Two versions of the sketch are available, one for sensor activated is HIGH and one for LOW.

-- 0 --
