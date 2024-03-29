// Ruud Boer, March 2022
// Reads the state if switch inputs every loop() cycle and stores the values in the
// sw[i] array, but only if the state was stable for at least DEBOUNCE ms.
// A state change is stored in the array sw-change[i], for the duration of
// one cycle. This can be used to do things only once after a switch changed state.

#define DEBOUNCE 100 // [ms]

int sw_pin[] = {8,9}; // Pin numbers of the switches. Table can have any length.

////////////////////////
// End of configuration
////////////////////////

const int numsw = sizeof(sw_pin) / sizeof(int);
int sw[numsw], sw_change[numsw]; // sw_change: H>L=0 L>H=1 none=2
unsigned long sw_time[numsw];

void setup() {
  for (byte i=0; i<numsw; i++)  pinMode(sw_pin[i],INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

void loop() {

//////////////////////////////////////////////////////////////////////////
// Read and store the state and transition of the switches, debounced
//////////////////////////////////////////////////////////////////////////
void read_sw() {
  for (byte i=0; i<numsw; i++) {
    byte reading = digitalRead(sw_pin[i]);
    if(reading==sw[i]) {
      sw_time[i] = millis();
      sw_change[i] = 2; // 2:no change
    }
    else if(millis() > sw_time[i] + (unsigned long)DEBOUNCE) {
      sw[i]        = reading; // 0=L, 1=H
      sw_change[i] = reading; // 0=HL 1=LH
    }
  }
}
//////////////////////////////////////////////////////////////////////////
// Do something when a switch state changed
//////////////////////////////////////////////////////////////////////////  
  if(sw_change[0]==0) Serial.println("SW0 = 0");
  if(sw_change[0]==1) Serial.println("SW0 = 1");
  if(sw_change[1]==0) Serial.println("SW1 = 0");
  if(sw_change[1]==1) Serial.println("SW1 = 1");
  
}
