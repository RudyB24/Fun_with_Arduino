// Ruud Boer, March 2022
// Pushbuttons are read via a function that returns:
// 2 if pressed longer than LONG_PRESS
// 1 if pressed longer than SHORT_PRESS
// 0 if pressed shorter than SHORT_PRESS, this also functions as a debounce

#define SHORT_PRESS  100 // [ms]
#define LONG_PRESS  2000 // [ms]

byte pb_pin[] = {4,5}; // Pin numbers of the pushbuttons

//////////////////////////////////////////////////////////////////////////
// End of configuration
//////////////////////////////////////////////////////////////////////////

const int numpb = sizeof(pb_pin);
byte pbstate[numpb], pressed;
unsigned long pb_time[numpb];

void setup() {
  for (byte i=0; i<numpb; i++)  pinMode(pb_pin[i],INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Started");
}

////////////////////////////////////////////////////////////////////
// Function: detect pushbuttons state change and return pushed time 
////////////////////////////////////////////////////////////////////
byte read_pb(byte i) {
  byte reading = digitalRead(pb_pin[i]);   // read the push button
  if(reading != pbstate[i]) {              // state has changed
    pbstate[i] = reading;
    if(!pbstate[i]) pb_time[i] = millis(); // HL transition, start timer
    else {                                 // LH transition, return push time
      int time_pushed = millis() - pb_time[i];
      if     (time_pushed > (int)LONG_PRESS)  return 2;
      else if(time_pushed > (int)SHORT_PRESS) return 1;
      else                                    return 0;
    }
  }
  else return 0;
} // End function read_pb()

void loop() {
//////////////////////////////////////////////////////////////////////////
// Read the pushbuttons and do something based on long or short press
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<numpb; i++) {
    pressed = read_pb(i); //0=not pressed, 1=short, 2=long
    if(pressed) { // Button has been pressed longer than SHORT_PRESS [ms]
      Serial.print("PB");
      Serial.print(i);
      if (pressed==1) Serial.println(" short");
      else            Serial.println(" long");
    }
  }
}
