Ruud Boer, April 2022

The DCC_Decoder folder is the library that has to be placed in the Arduino library folder. The library is used by the DCC sketches to decode the DCC data, besides the DCC_sniffer which has its own internal DCC decoding.

The Arduino sketches DCC_Decoder_Servo and DCC_Decoder_Accessory read model railway DDC data. The data is fed to interrupt pin 2 via an 6N137 fast opto coupler according to this schematic: https://rudysmodelrailway.files.wordpress.com/2014/07/schematic1.png

If an accessory switch command is detected for one of the DCC addresses that is declared in the code, then it moves the attached servo to the min or max angle, which are also declared in the code, or it switches the accessory output pin on/off.

-- 0 --
