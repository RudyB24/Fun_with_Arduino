// Ruud Boer, March 2022
// Pushbuttons are read in a function that returns:
// 2 if pressed longer than LONG_PRESS
// 1 if pressed longer than SHORT_PRESS
// 0 if pressed shorter than SHORT_PRESS

#define SHORT_PRESS  100 // [ms]
#define LONG_PRESS  2000 // [ms]

int pb_pin[] = {10,11};  // Pin numbers of the pushbuttons

////////////////////////
// End of configuration
////////////////////////

const int numpb = sizeof(pb_pin)/sizeof(int);
int pb[numpb], pb_state[numpb]; // pushbutton states
unsigned long pb_time[numpb];   // pushbutton timers

void setup() {
  for (byte i=0; i<numpb; i++)  pinMode(pb_pin[i],INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

///////////////////////////////////////////////////////////////////////////////
// Function: detect pushbutton presses and store 0=short-, 1=long-, 2=no press
///////////////////////////////////////////////////////////////////////////////
void read_pb() {
  for (int i=0; i<numpb; i++) {  
    int reading = digitalRead(pbpin[i]); // read the push button
    if(reading != pbstate[i]) {          // state has changed
      pbstate[i] = reading;
      if(!reading) pbtime[i] = millis(); // HL transition, start timer
      else {                             // LH transition, store push time
        int timepushed = millis() - pbtime[i];
        if     (timepushed > (int)LONG_PRESS)  pb[i] = 1; // value is only here for one loop cycle
        else if(timepushed > (int)SHORT_PRESS) pb[i] = 0; // value is only here for one loop cycle
        else                                   pb[i] = 2; // pressed too short
      }
    }
    else pb[i] = 2; // not pressed
  }
}

void loop() {
  if(pb[0]==1) Serial.println("PB0 short");  
  if(pb[0]==2) Serial.println("PB0 long");
  if(pb[1]==1) Serial.println("PB1 short");
  if(pb[1]==2) Serial.println("PB1 long");
}
