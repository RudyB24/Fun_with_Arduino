**Read pushbuttons inputs**

The pin numbers of the pushbutton inputs are configured in the pb_pin[i] array.

The pushbuttons are read in a function that returns:
- 2 if pressed longer than LONG_PRESS ms
- 1 if pressed longer than SHORT_PRESS ms
- 0 if pressed shorter than SHORT_PRESS ms, this automatically functions as debounce

Pushbutton states are stored in the pb[i] array.  

-- 0 --
