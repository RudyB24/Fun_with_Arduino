// Ruud Boer - March 2022

//--------------------------------------------------------------------------------------------------
// In te vullen waarden:

//                        RRR,GGG,BBB,WWW       light_state
byte light_state[5][4] ={{255,255,255,255},  // 0 Work light
                         {255,  0,  0,  0},  // 1 Day
                         {  0,255,  0,  0},  // 2 Evening
                         {  0,  0,255,  0},  // 3 Night
                         {  0,  0,  0,255}}; // 4 Morning

int fade_interval = 10; // [ms] the higher the number, the slower the fade. Value 10 equals 2,5 sec.

unsigned int light_state_time[] = {10,4,10,4}; // [s] day, evening, night, morning 

//--------------------------------------------------------------------------------------------------

// Control for Model Railroud Layout LED lightins
// User defined states, defined by RGBW values:
// 0 Work light
// 1 Day
// 2 Evening
// 3 Night
// 4 Morning

// Inputs (switch to GND) are used to:
// - Switch to one of the 5 states
// - Start sequencer

byte i, sequencer, state, test, R,G,B,W, Rtarget, Gtarget, Btarget, Wtarget;

//                   0,1,2,3,4,5   // light_state number
byte TC_input[]   = {2,3,4,5,7,8}; // TrainController inputs (pushbuttons)

byte LED_output[] = {6,9,10,11};



unsigned long time_to_change_state, time_for_fade_step;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (i = 0; i < 6; i++) {
    pinMode(TC_input[i], INPUT_PULLUP);
  }
  Serial.begin(9600);
  Serial.println("Started");
  setRGBW(0);
}

void setRGBW(byte s) {
  Rtarget = light_state[s][0];
  Gtarget = light_state[s][1];
  Btarget = light_state[s][2];
  Wtarget = light_state[s][3];
}

void loop() {

  for (i = 0; i < 6; i++) {
    if(!digitalRead(TC_input[i])) {
      if(i==5) {
        sequencer = 1;
        state = 4;
        Serial.println("Sequencer started");
      }
      else {
        sequencer = 0;
        state = i;
        setRGBW(state);
        Serial.print("State = ");
        Serial.println(state);
      }
      delay(500);
    }
  }
    
  if(sequencer) {
    switch (state) {
      case 1:
        setRGBW(1);
        if(millis() > time_to_change_state) {
          state = 2;
          time_to_change_state = millis() + (unsigned long)light_state_time[1] * 1000UL;
        }
      break;
      case 2:
        setRGBW(2);
        if(millis() > time_to_change_state) {
          state = 3;
          time_to_change_state = millis() + (unsigned long)light_state_time[2] * 1000UL;      
        }
      break;
      case 3:
        setRGBW(3);
        if(millis() > time_to_change_state) {
          state = 4;
          time_to_change_state = millis() + (unsigned long)light_state_time[3] * 1000UL;      
        }
      break;
      case 4:
        setRGBW(4);
        if(millis() > time_to_change_state) {
          state = 1;
          time_to_change_state = millis() + (unsigned long)light_state_time[0] * 1000UL;
        }
      break;
    } // end switch
  }   // end if

  if(millis() > time_for_fade_step) {
    time_for_fade_step = millis() + fade_interval;
    if(R<Rtarget) R++; if(R>Rtarget) R--;
    if(G<Gtarget) G++; if(G>Gtarget) G--;
    if(B<Btarget) B++; if(B>Btarget) B--;
    if(W<Wtarget) W++; if(W>Wtarget) W--;
}
  analogWrite(LED_output[0], R);
  analogWrite(LED_output[1], G);
  analogWrite(LED_output[2], B);
  analogWrite(LED_output[3], W);
}
