// Ruud Boer - April 2022
// Switch RGBW LED lighting to one of 9 states, with a fade between states
// States each with their own RGBW values:
// 0 Day light room
// 1 Evening light room
// 2 Night light room
// 3 Morning light room
// 4 Day dark room
// 5 Evening dark room
// 6 Night dark room
// 7 Morning dark room
// 8 Work light

//--------------------------------------------------------------------------------------------------
// Data to be filled in:
//--------------------------------------------------------------------------------------------------

#define FILTERTIME 500 // [ms] Switch inputs must be stable FILTERTIME ms before being accepted

byte TC_input_pin[]   = {2,3,4,5,7,8,12}; // Inputs (permanent switches), active 0, LOW, GND.
// 0 = pin  2 = Day
// 1 = pin  3 = Evening
// 2 = pin  4 = Night
// 3 = pin  5 = Morning
// 4 = pin  7 = Sequencer
// 5 = pin  8 = Work Light
// 6 = pin 12 = Change between setpoints for light or dark room

byte LED_output_pin[] = {6,9,10,11};   // Must be PWM outputs marked with ~

//                        RRR,GGG,BBB,WWW       state
byte light_state[9][4] ={{  0,  0,  0,255},  // 0 Day     light room
                         {255,  0,  0,  0},  // 1 Evening light room
                         {  0,255,  0,  0},  // 2 Night   light room
                         {  0,  0,255,  0},  // 3 Morning light room
                         {  0,  0,  0, 50},  // 4 Day     dark room
                         { 50,  0,  0,  0},  // 5 Evening dark room
                         {  0, 50,  0,  0},  // 6 Night   dark room
                         {  0,  0, 50,  0},  // 7 Morning dark room
                         {255,255,255,255}}; // 8 Work Light

int sequencer_time[]   = {9,9,9,9}; // [s] day, evening, night, morning
int fade_interval      = 20; // [ms] Higher number = slower fade, 10 equals 2.5 s
int fade_interval_wl   = 10; // [ms] Separate (faster) fade time for Work Light

//--------------------------------------------------------------------------------------------------
// End of configuration. Don't change anything below this line
//--------------------------------------------------------------------------------------------------

const int numinputs = sizeof(TC_input_pin); // How many inputs are in use with this Arduino
byte i, state, state_old, sequencer, R,G,B,W, Rtarget, Gtarget, Btarget, Wtarget;
byte sw[numinputs];
unsigned long time_to_change_state, time_for_fade_step, sw_time[numinputs];

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (i = 0; i < numinputs; i++) {
    pinMode(TC_input_pin[i], INPUT_PULLUP);
    sw[i] = 1;
  } 
  state = 8; // Start with Work Light
  Serial.begin(9600);
  Serial.println(numinputs);
  Serial.println("Ready");
}

void loop() {
  
  for (byte i=0; i<numinputs; i++) { // Read the switches with anti interference FILTERTIME
    byte reading = digitalRead(TC_input_pin[i]);
    if(reading==sw[i]) {
      sw_time[i] = millis();
    }
    else if(millis() > sw_time[i] + (unsigned long)FILTERTIME) {
      sw[i] = reading;
    }
  }

  if(!sw[4]) {
    sequencer = 1; // Sequencer enabled
    digitalWrite(13, 1);
  }
  else if(!sw[5]) {
    sequencer = 0;
    digitalWrite(13, 0);
    state = 8;
  }
  else {
    sequencer = 0; // Sequencer stopped
    digitalWrite(13, 0);
    state = 8;     // Work Light if no switch is active
    for (i = 0; i < 4; i++) { // Check switch inputs 0-3
      if(!sw[i]) state = i;
    }
  }
   
  if(sequencer) {
    if(millis() > time_to_change_state) {
      state = state + 1;       // Go to next state
      if(state > 3) state = 0; // After 'morning', restart cycle with 'day'
      time_to_change_state = millis() + sequencer_time[state] * 1000UL;
    }
  }
  
  if(state!=8 && !sw[6]) state = state + 4; // Set state to 4-7 for dark room setpoints
  if(state != state_old) {
    state_old = state; 
    Serial.println(state);
    Rtarget = light_state[state][0];
    Gtarget = light_state[state][1];
    Btarget = light_state[state][2];
    Wtarget = light_state[state][3];
  }
  if(state!=8 && !sw[6]) state = state - 4; // Set state back to 0-3

  if(millis() > time_for_fade_step) {
    if(state==8) time_for_fade_step = millis() + (unsigned long)fade_interval_wl;
    else         time_for_fade_step = millis() + (unsigned long)fade_interval;
    if(R<Rtarget) R++; if(R>Rtarget) R--;
    if(G<Gtarget) G++; if(G>Gtarget) G--;
    if(B<Btarget) B++; if(B>Btarget) B--;
    if(W<Wtarget) W++; if(W>Wtarget) W--;
    analogWrite(LED_output_pin[0], R);
    analogWrite(LED_output_pin[1], G);
    analogWrite(LED_output_pin[2], B);
    analogWrite(LED_output_pin[3], W);
  }
}
