// Ruud Boer - March 2022
// Reads digital encoder Data, Clk, Switch inputs
// Sends out a pwm signal to a FET to control a LED strip
// state1: fade to brightness[0], sensor active, short press >> state3, long press >> state3
// state1: if detector sees nothing, after STAY_ON_TIME the LEDs turn off
// state2: brightness[1], sensor not active so LEDs stay on, short press >> state1, long press >> state3
// state3: transition state to write brightness values to EEPROM and go to state4
// state4: LEDs off. Short press >> state1, long press >> state2
// brightnesses[0] and [1] can be changed by rotating the encoder. They are stored in EEPROM in state3

#define STAY_ON_TIME    60 // [s] fade off after ... seconds
#define SHORT_PRESS     20 // [ms] short press
#define LONG_PRESS    1500 // [ms] long  press
#define FADE_INTERVAL   30 // [ms] time between LED fade steps
#define BRIGHT_A       200 // PWM value (range: 0-255) in sensor state
#define BRIGHT_B        50 // PWM value (range: 0-255) in permanent state
#define ENC_DEBOUNCE    10 // [ms] encoder debounce
#define ENC_SLOW_FAST  250 // [ms] encoder slow / fast rotation limit

// PIN NUMBERS REAL UNIT
#define ENC_A_PIN      2 // rotary encoder CLK or A
#define ENC_B_PIN      3 // rotary encoder DATA or B
#define ENC_SW_PIN     4 // rotary encoder SWITCH, button pressed = LOW
#define SENSOR_PIN     6 // HIGH = movement detected
#define PWM_OUTPUT_PIN 5 // must be one of the pwm pins: 3,5,6,9,10,11

/*
// PIN NUMBERS DEMO BOX
#define ENC_A_PIN     14 // rotary encoder CLK or A
#define ENC_B_PIN     15 // rotary encoder DATA or B
#define ENC_SW_PIN     2 // rotary encoder SWITCH, button pressed = LOW
#define SENSOR_PIN     7 // HIGH = movement detected
#define PWM_OUTPUT_PIN 5 // must be one of the pwm pins: 3,5,6,9,10,11
*/

#include <EEPROM.h>
#define LED_PIN 13
byte pwm_value, pwm_target;         // PWM value for LED brightness
byte state; // 1=sensor active, 2=continuous 3=transition 4=off
byte pb, pb_old, pushb, pushb_old;  // encoder pushbutton
byte encA, encA_old, encB;          // encoder values
int bright[2], bright_old[2];       // LED brightness [0]=sensor [1]=permanent
int enc_value, enc_value_old;
unsigned long enc_rot_time, pb_time, timetofade, timetoswitchoff;

//////////////////////////////////////////////////////////////////////////
// Function: detect pushbuttons state change and return pushed time 
//////////////////////////////////////////////////////////////////////////
byte read_pb() {
  pb = digitalRead(ENC_SW_PIN); // read the push button
  if(pb != pb_old) {            // state has changed
    pb_old = pb;
    if(pb==0) pb_time = millis(); // 1 > 0 transition, start timer
    else {                        // 0 > 1 transition, return push time
      int time_pushed = millis() - pb_time;
      if     (time_pushed > (int)LONG_PRESS)  return 2;
      else if(time_pushed > (int)SHORT_PRESS) return 1;
      else                                    return 0;
    }
  }
  else return 0;
}

//////////////////////////////////////////////////////////////////////////
// Function: determine encoder slow or fast rotation
//////////////////////////////////////////////////////////////////////////
int read_encoder() {
  int increment;
  encA = digitalRead(ENC_A_PIN);
  if((encA_old == 0) && (encA == 1)) { // 0 > 1 transition
    if     ((millis() - enc_rot_time) > ENC_SLOW_FAST) increment =  1;
    else if((millis() - enc_rot_time) > ENC_DEBOUNCE)  increment = 10;
    else                                               increment =  0;
    enc_rot_time = millis();
    encA_old = encA;
    if(digitalRead(ENC_B_PIN) == 1) return  increment;
    else                            return -increment;
  }
  encA_old = encA;
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Function: determine encoder slow or fast rotation
//////////////////////////////////////////////////////////////////////////
void bright_adjust(byte i) {
  bright[i] = bright[i] + read_encoder();
  if (bright[i] < 0)   bright[i] =   0;
  if (bright[i] > 255) bright[i] = 255;
  if(bright[i]!=bright_old[i]) {
    Serial.println(bright[i]);
    bright_old[i] = bright[i];
  }
}

void setup() { 
	pinMode (ENC_A_PIN,INPUT_PULLUP);
	pinMode (ENC_B_PIN,INPUT_PULLUP);
	pinMode (ENC_SW_PIN,INPUT_PULLUP);
	pinMode (SENSOR_PIN,INPUT_PULLUP);
	pinMode (PWM_OUTPUT_PIN,OUTPUT);
	pinMode (LED_PIN,OUTPUT);
  
  pinMode ( 9,OUTPUT);
  pinMode (10,OUTPUT);

	bright[0] = EEPROM.read(0);
	bright[1] = EEPROM.read(1);
  state = 1;
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

void loop() {
  pushb = read_pb();

  switch (state) {
		case 1: // bright[0] and sensor is active
			// encoder: adjust brightness
			// start timer when sensor does not detect movement, if timer elapsed: brightness = 0
			// short press >> state2: always on
			// long press >> state3: off
      bright_adjust(0);
			if (digitalRead(SENSOR_PIN)) { // SENSOR_PIN = HIGH when motion is detected
				timetoswitchoff = millis() + (unsigned long)STAY_ON_TIME * 1000;
			  // timetoswitchoff keeps shifting as long as the sensor detects motion (=HIGH)
				pwm_target = bright[0];
			}
			if (millis() > timetoswitchoff) pwm_target = 0; // no movement detected and STAY_ON_TIME has passed
			if (pushb==2) { // long press: go to state 3: off
        state = 3;
        Serial.println("state 3");
      }
			if (pushb==1) { // short press: go to state2: always on
        state = 2;
        Serial.println("state 2");
        delay(200);
      }
			digitalWrite(LED_PIN,LOW);
		break;
	
		case 2: // bright2, sensor is not active, LEDs stay on permanently
			// encoder: adjust brightness
			// short press >> state1: sensor active
			// long press >> state0: off
      bright_adjust(1);
      pwm_target = bright[1];
			if (pushb==2) { // long press: go to state 3: off
        state = 3;
        Serial.println("state 3");
      }
			if (pushb==1) { // short press: go to state1: sensor
        state = 1;
        Serial.println("state 1");
        delay(200);
      }
			digitalWrite(LED_PIN,HIGH);
		break;

		case 3: // write brightnesses to EEPROM and goto state 0
			EEPROM.write(0, (byte)bright[0]);
			EEPROM.write(1, (byte)bright[1]);
			state = 4;
      Serial.println("state 4");
      pwm_target = 0;
      digitalWrite(LED_PIN,LOW);
		break;

    case 4: // LEDs off, sensor not active
			if(pushb>0) state = pushb;
      if(state!=4) {Serial.print("state "); Serial.println(state);}
		break;
	} // End switch(state)

	if (millis() > timetofade) {
		timetofade = millis() + (unsigned long)FADE_INTERVAL;
		if (pwm_value < pwm_target) pwm_value++;
		if (pwm_value > pwm_target) pwm_value--;
		analogWrite(PWM_OUTPUT_PIN, pwm_value);
	}	
} // End loop()