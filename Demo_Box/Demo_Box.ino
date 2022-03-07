// Ruud Boer, March 2022
// Demo box with LEDs, pushbuttons, switches, potmeter, encoder, servo

// BEWARE: If servo.h in included, pins 9,10 do NOT work as analogWrite()

#define FADE_INTERVAL 2500 // [us]
#define FADE_LOW         1 // PWM value 0-255
#define FADE_HIGH      255 // PWM value 0-255
#define SERVO_INTERVAL  30 // [ms] time between servo steps, larger is slower
#define SERVO_MIN        5 // servo angle 0-180 degrees
#define SERVO_MAX      175 // servo angle 0-180 degrees
#define ROTARY_FAST     10 // [ms] rotary fast rotation: change value by 10
#define ROTARY_SLOW    200 // [ms] rotary slow rotation: change value by  1

#define BLINK_ON        50 // [ms]
#define BLINK_OFF      450 // [ms]
#define DEBOUNCE      1000 // [ms]
#define SHORT_PRESS    100 // [ms]
#define LONG_PRESS    2000 // [ms]

#define ENCA_PIN        14 // A0 Encoder A or Data
#define ENCB_PIN        15 // A1 Encoder B or Clock
#define POTM_PIN        17 // A3 Potentiometer Analog In
#define SERVO_PIN       18 // A4 Servo

#define LED_FADE         0 // number of the LED to use for fade
#define LED_ENCODER      1 // number of the LED to use for the encoder value
#define LED_POTMETER     2 // number of the LED to use for the potmeter analogRead value

byte sw_pin[]  = {7,8};      // pin numbers of the switches
byte pb_pin[]  = {2,4,16};   // pin numbers of the pushbuttons, 16 is the encoder-sw
byte pwm_pin[] = {3,5,6,11}; // pin numbers of the PWM outputs (don't use 9,10 with servo.h)
byte dig_pin[] = {9,10,13};  // pin numbers of the digital outputs

//////////////////////////////////////////////////////////////////////////
// End of configuration
//////////////////////////////////////////////////////////////////////////

const int numsw  = sizeof(sw_pin);
const int numpb  = sizeof(pb_pin);
const int numpwm = sizeof(pwm_pin);
const int numdig = sizeof(dig_pin);
byte sw[numsw], sw_old[numsw], sw_change[numsw]; // switches, sw_change: H>L=0 L>H=1 none=2
byte pb[numpb], pb_old[numpb];                   // pushbuttons
byte servo_value, servo_target;
byte encA, encA_old, encB, encB_old;
byte blinking, pushed;
byte pwm_value[numpwm], pwm_target[numpwm], dig_value[numdig]; 
int analog_reading, enc_value, enc_value_old;
unsigned int st = 1;  // states & transitions
unsigned long timetofade, timetoblink, timetomoveservo, enc_rot_time;
unsigned long sw_time[numsw], pb_time[numpb];

#include <Servo.h>  // Servo functions are now included
Servo servo;        // A Servo is declared with the name servo

void setup() {
  pinMode(ENCA_PIN, INPUT_PULLUP);
  pinMode(ENCB_PIN, INPUT_PULLUP);
  for (byte i=0; i<numpb; i++)  pinMode(pb_pin[i],INPUT_PULLUP);
  for (byte i=0; i<numsw; i++)  pinMode(sw_pin[i],INPUT_PULLUP);
  for (byte i=0; i<numdig; i++) pinMode(dig_pin[i],OUTPUT);
  // pwm_pin[] are not specified as OUTPUT, they are analogWrite() PWM pins
  servo.attach(SERVO_PIN,1000,2000); //servo connected (pin, [us] pulse min, [us] pulse max)
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

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
}

//////////////////////////////////////////////////////////////////////////
// Function: determine encoder slow or fast rotation
//////////////////////////////////////////////////////////////////////////
int read_encoder() {
  int increment;
  encA = digitalRead(ENCA_PIN);
  if((encA_old == 0) && (encA == 1)) { // 0 > 1 transition
    if     ((millis() - enc_rot_time) > ROTARY_SLOW) increment =  1;
    else if((millis() - enc_rot_time) > ROTARY_FAST) increment = 10;
    else                                                 increment =  0;
    enc_rot_time = millis();
    encA_old = encA;
    if(digitalRead(ENCB_PIN) == 1) return  increment;
    else                           return -increment;
  }
  encA_old = encA;
  return 0;
}

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

//////////////////////////////////////////////////////////////////////////
// Read and store if pushbuttons are pressed, short or long
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<numpb; i++) pb[i] = read_pb(i);

//////////////////////////////////////////////////////////////////////////
// Example of state transition diagram, st<100: state, st>100: transition
//////////////////////////////////////////////////////////////////////////
  switch(st) {
    case 1: // read pushbuttons (pb) and switches (sw)
      if(pb[0]==1) st = 101;  
      if(pb[0]==2) st = 102;
      if(pb[1]==1) dig_value[1] = 1;
      if(pb[1]==2) dig_value[1] = 0;
      if(sw_change[0]==0) Serial.println("Fade off");
      if(sw_change[0]==1) Serial.println("Fade on");
      if(sw_change[1]==0) Serial.println("Blink off");
      if(sw_change[1]==1) Serial.println("Blink on");
      if(sw[0]==1) pwm_target[LED_FADE] = FADE_HIGH;
      else         pwm_target[LED_FADE] = FADE_LOW;
      if(sw[1]==1) blinking = 1;
      else         blinking = 0;
    break;
    case 101:
      if(servo_target==SERVO_MIN) servo_target = SERVO_MAX;
      else                        servo_target = SERVO_MIN;
      Serial.print("Servo target = ");
      Serial.println(servo_target);
      st = 1;
    break;
    case 102:
      servo_target = 90;
      Serial.print("Servo target = ");
      Serial.println(servo_target);
      st = 1;
    break;
  } // end switch

//////////////////////////////////////////////////////////////////////////
// Blink the LED on dig_pin[0] with different on and off times
//////////////////////////////////////////////////////////////////////////
  if (blinking==1) {
    if (millis() > timetoblink) {
      if(dig_value[0]==0) { // LED is off, we turn it on now   
        timetoblink = millis() + (unsigned long)BLINK_ON;
        dig_value[0] = 1;
      }
      else {                // led is on, we turn it off now
        timetoblink = millis() + (unsigned long)BLINK_OFF;
        dig_value[0] = 0;
      }
    }
  }
  else dig_value[0] = 0;
  dig_value[2] = dig_value[0]; //  Also blink the on board LED on pin 13

//////////////////////////////////////////////////////////////////////////
// Update a LED value if it's time, this creates a fade
//////////////////////////////////////////////////////////////////////////
  if (micros() > timetofade) {
      timetofade = micros() + (unsigned long)FADE_INTERVAL;
      if(pwm_value[LED_FADE] < pwm_target[LED_FADE]) pwm_value[LED_FADE]++;
      if(pwm_value[LED_FADE] > pwm_target[LED_FADE]) pwm_value[LED_FADE]--;
  }

//////////////////////////////////////////////////////////////////////////
// Move the servo a step closer to the target angle if it's time
//////////////////////////////////////////////////////////////////////////
  if (millis() > timetomoveservo) {
    timetomoveservo = millis() + (unsigned long)SERVO_INTERVAL;
    if(servo_value < servo_target) servo_value++;
    if(servo_value > servo_target) servo_value--;
    servo.write(servo_value);
  }

//////////////////////////////////////////////////////////////////////////
// Read the analog input and change LED_POTMETER brightness accordingly
//////////////////////////////////////////////////////////////////////////
  analog_reading = analogRead(POTM_PIN);
  pwm_value[LED_POTMETER] = analog_reading / 4;
  
//////////////////////////////////////////////////////////////////////////
// Read the digital encoder and change LED_ENCODER brightness accordingly
//////////////////////////////////////////////////////////////////////////  
  enc_value = enc_value + read_encoder();
  if(enc_value > 255) enc_value = 255;
  if(enc_value <   0) enc_value =   0;
  if(enc_value != enc_value_old) {
    pwm_value[LED_ENCODER] = enc_value;
    Serial.println(enc_value);
    enc_value_old = enc_value;
  }

//////////////////////////////////////////////////////////////////////////
// Write the PWM and the digital values to the outputs
////////////////////////////////////////////////////////////////////////// 
  for (byte i=0; i<numpwm; i++) analogWrite(pwm_pin[i],pwm_value[i]);
  for (byte i=0; i<numdig; i++) digitalWrite(dig_pin[i],dig_value[i]);
} // end loop
