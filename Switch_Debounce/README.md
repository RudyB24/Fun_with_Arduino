**Read toggle switch inputs**

The pin numbers of the switch inputs are configured in the sw_pin[i] array.

Their states are read every loop() cycle and are stored in the sw[i] array.

The inputs are debounced, the sw[i] only schange state if the state is stable longer than DEBOUNCE ms.

A state change from 0 -> 1 or from 1 -> 0 is stored in the sw_change[i] array for the duration of one cycle. This can be used to do something only once after the switch state has changed.
