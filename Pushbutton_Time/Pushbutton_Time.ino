  // Ruud Boer, March 2022
// Pushbuttons are read in a function that returns:
// - 2 if pressed longer than LONG_PRESS
// - 1 if pressed longer than SHORT_PRESS
// - 0 if pressed shorter than SHORT_PRESS, this also functions as a debounce

#define SHORT_PRESS  100 // [ms]
#define LONG_PRESS  2000 // [ms]

byte pb_pin[] = {2,4};  // Pin numbers of the pushbuttons. Table can have any length. 

//////////////////////////////////////////////////////////////////////////
// End of configuration
//////////////////////////////////////////////////////////////////////////

const int numpb  = sizeof(pb_pin);
byte pb[numpb], pb_old[numpb]; // pushbutton states
unsigned long pb_time[numpb];  // pushbutton timers

void setup() {
  for (byte i=0; i<numpb; i++)  pinMode(pb_pin[i],INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
} // End setup()

//////////////////////////////////////////////////////////////////////////
// Function: detect pushbuttons state change and return pushed time 
//////////////////////////////////////////////////////////////////////////
byte read_pb(byte i) {
  pb[i] = digitalRead(pb_pin[i]); // read the push button
  if(pb[i] != pb_old[i]) {        // state has changed
    pb_old[i] = pb[i];
    if(pb[i]==0) pb_time[i] = millis(); // 1 > 0 transition, start timer
    else {                              // 0 > 1 transition, return push time
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
// Read if pushbuttons are pressed short or long
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<numpb; i++) pb[i] = read_pb(i);
  if(pb[0]==1) Serial.println("PB0 short");  
  if(pb[0]==2) Serial.println("PB0 long");
  if(pb[1]==1) Serial.println("PB1 short");
  if(pb[1]==2) Serial.println("PB1 long");
} // End loop
