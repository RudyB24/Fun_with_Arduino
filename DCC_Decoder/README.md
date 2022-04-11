Ruud Boer, April 2022

This Arduino sketch reads model railway DDC data that is fed to interrupt pin 2 via an N306 fast opto coupler.

If an accessory switch command is detected for one of its DCC addresses, which are declared in the code, then it moves the attached servo to the min or max angle, which are also declared in the code.

The DCC_Decoder library is needed for this sketch to work. Place that folder in the Arduino installation library folder.
