#define DEBOUNCE      1000 // [ms]

byte sw_pin[]  = {7,8};      // Pin numbers of the switches. Table can have any length.

//////////////////////////////////////////////////////////////////////////
// End of configuration
//////////////////////////////////////////////////////////////////////////

const int numsw  = sizeof(sw_pin);
byte sw[numsw], sw_old[numsw], sw_change[numsw]; // switches, sw_change: H>L=0 L>H=1 none=2

void setup() {
  for (byte i=0; i<numsw; i++)  pinMode(sw_pin[i],INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
} // End setup()

void loop() {

//////////////////////////////////////////////////////////////////////////
// Read and store the state and transition of the switches, debounced
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<numsw; i++) {
    byte reading = digitalRead(sw_pin[i]);
    if(sw[i]==reading) {
      sw_time[i] = millis();
      sw_change[i] = 2; // 2 = no change
    }
    else if(millis() > sw_time[i] + (unsigned long)DEBOUNCE) {
      sw[i]        = reading;
      sw_change[i] = reading;
    }
  } // end for
  
  if(sw_change[0]==0) Serial.println("Sw0 = 0");
  if(sw_change[0]==1) Serial.println("Sw0 = 1");
  if(sw_change[1]==0) Serial.println("Sw1 = 0");
  if(sw_change[1]==1) Serial.println("Sw1 = 1");
  
} // End loop()
