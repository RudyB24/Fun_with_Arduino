## Railway Crossing Multi Track 2 Way 2 Gates

This Arduino sketch controls a model railway crossing.

Two servos are used to close / open the gates.

Two digital outputs are used to blink LEDs.

Per track two sensors are used to detect an arriving train and a departing train. The arrival sensor must be far enough away to enable the gates to close before the train reaches the crossing. The departure sensor is located close to the crossing, a timer is used to decide when the tail of the train has passed.

Internally a counter is used that counts up with every arrival of a train and counts down with every departure. This assures the gates stay closed, or re-close while they were already opening, when trains on different tracks are detected via the arrival sensors.

[More info over here.](https://rudysarduinoprojects.wordpress.com/2020/09/23/fun-with-arduino-42-railway-crossing-multi-track-two-way/)

-- 0 --
