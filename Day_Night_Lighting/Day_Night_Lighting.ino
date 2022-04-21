 // Ruud Boer - April 2022
// Control for RGBW LED lighting
// Five lightings states each have their own RGBW values:
// 0 Day
// 1 Evening
// 2 Night
// 3 Morning
// 4 Work light

// 4 inputs (switch. not pushbutton, active when GND) are used to enable one of the states 0-3
// A 5th input switches a sequencer that steps from one state to the next via a gradual fade
// If no input is present the state is 4: Work Light

// Data to be filled in:
//--------------------------------------------------------------------------------------------------
#define FILTERTIME 500 // [ms] Switch inputs must be stable FILTERTIME ms before being accepted

byte TC_input_pin[]   = {2,3,4,5,8}; // Inputs (permanent switches, not pushbuttons)
byte LED_output_pin[] = {6,9,10,11}; // Must be PWM outputs marked with ~

//                        RRR,GGG,BBB,WWW       state
byte light_state[5][4] ={{  0,  0,  0,255},  // 0 Day
                         {255,  0,  0,  0},  // 1 Evening
                         {  0,255,  0,  0},  // 2 Night
                         {  0,  0,255,  0},  // 3 Morning
                         {255,255,255,255}}; // 4 Work Light

int sequencer_time[]   = {9,9,9,9}; // [s] day, evening, night, morning
int fade_interval      = 20; // [ms] Higher number = slower fade, 10 equals 2.5 s
int fade_interval_wl   = 10; // [ms] Separate (faster) fade time for Work Light
//--------------------------------------------------------------------------------------------------

byte i, state, state_old, sequencer, R,G,B,W, Rtarget, Gtarget, Btarget, Wtarget;
byte sw[5];
unsigned long time_to_change_state, time_for_fade_step, sw_time[5];

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (i = 0; i < 5; i++) {
    pinMode(TC_input_pin[i], INPUT_PULLUP);
    sw[i] = 1;
  } 
  state = 4; // Start with Work Light
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop() {
  
  for (byte i=0; i<5; i++) { // Read the switches with anti interference FILTERTIME
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
  else {
    sequencer = 0; // Sequencer stopped
    digitalWrite(13, 0);
    state = 4;     // Work Light if no switch is active
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

  if(state != state_old) {
    Serial.println(state);
    state_old = state;
    Rtarget = light_state[state][0];
    Gtarget = light_state[state][1];
    Btarget = light_state[state][2];
    Wtarget = light_state[state][3];
  }

  if(millis() > time_for_fade_step) {
    if(state==4) time_for_fade_step = millis() + (unsigned long)fade_interval_wl;
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
