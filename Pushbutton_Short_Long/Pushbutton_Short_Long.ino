// Ruud Boer, March 2022
// Pushbuttons that make in input LOW are read via a function that returns:
// 0 if pressed longer than SHORT_PRESS
// 1 if pressed longer than LONG_PRESS
// 2 if pressed shorter than SHORT_PRESS, which is regareded no press

#define SHORT_PRESS  100 // [ms]
#define LONG_PRESS  2000 // [ms]

byte pbpin[] = {4,5}; // Pin numbers of the pushbuttons

////////////////////////
// End of configuration
////////////////////////

const int numpb = sizeof(pbpin);
byte pbstate[numpb];
unsigned long pbtime[numpb];

void setup() {
  for (byte i=0; i<numpb; i++)  pinMode(pbpin[i], INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Started");
}

///////////////////////////////////////////////////////////////////////////////////
// Function: detect pushbuttons state change and store 0=short, 1=long, 2=no press 
///////////////////////////////////////////////////////////////////////////////////
byte read_pb(byte i) {
  byte reading = digitalRead(pbpin[i]); // read the push button
  if (reading != pbstate[i]) {           // state has changed
    pbstate[i] = reading;
    if (!reading) pbtime[i] = millis();  // HL transition, start timer
    else {                              // LH transition, store push time
      int timepushed = millis() - pbtime[i];
      if      (timepushed > (int)LONG_PRESS)  return 1; // value is only here for one loop cycle
      else if (timepushed > (int)SHORT_PRESS) return 0; // value is only here for one loop cycle
      else                                    return 2; // not pushed
    }
  }
  else return 2; // not pushed
}

void loop() {
  for (byte i=0; i<numpb; i++) {
    if (read_pb(i) != 2) { // 0=short, 1=long, 2=no press
      Serial.print("PB");
      Serial.print(i);
      if (!pb[i]) Serial.println(" short"); // 0=short press
      else        Serial.println(" long");  // 1= long press
    }
  }
}
