**Read digital encoder**  
A digital encoder is read in a function that returns:
- 10 if rotation was faster than ENC_SLOW_FAST ms
- 1 if rotation was slower than ENC_SLOW_FAST ms
- 0 if pulses were shorter than ENC_DEBOUNCE ms
