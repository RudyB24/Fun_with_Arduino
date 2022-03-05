// Ruud Boer, March 2022
// Demo box with LEDs, pushbuttons, switches, potmeter, encoder, servo

#define FADE_INTERVAL 2500 // [us]
#define FADE_LOW         1 // PWM value 0-255
#define FADE_HIGH      255 // PWM value 0-255
#define SERVO_INTERVAL  30 // [ms] time between servo steps, larger is slower
#define SERVO_MIN        5 // servo angle 0-180 degrees
#define SERVO_MAX      175 // servo angle 0-180 degrees
#define FADE_HIGH      255 // PWM value 0-255
#define ROTARY_FAST     10 // [ms] rotary fast rotation: change value by 10
#define ROTARY_SLOW    200 // [ms] rotary slow rotation: change value by  1

#define blink_LED        0 // number of the LED used for blinking
#define fade_LED         1 // number of the LED used for fade
#define encoder_LED      2 // number of the LED used for the encoder value

#define BLINK_ON        50 // [ms]
#define BLINK_OFF      450 // [ms]
#define DEBOUNCE      1000 // [ms]
#define SHORT_PRESS    100 // [ms]
#define LONG_PRESS    2000 // [ms]
#define NUMSW            2 // Number of SWitches to read out
#define NUMPB            2 // Number of pushbuttons to read out

#define ENCA_pin  14 // A0 Encoder A or Data
#define ENCB_pin  15 // A1 Encoder B or Clock
#define POTM_pin  17 // A3 Potentiometer Analog In
#define SERVO_pin 18 // A4 Servo

byte SW_pin[]  = {7,8};    // pin numbers of the switches (AND IN FUTURE: encoder)
byte PB_pin[]  = {2,4,16}; // pin numbers of the pushbuttons, 16 is encoder-SW
byte LED_pin[] = {3,5,6,9,10,11}; // pin numbers of the LED PWM outputs

//////////////////////////////////////////////////////////////////////////
// End of configuration
//////////////////////////////////////////////////////////////////////////

const int numSW  = sizeof(SW_pin);
const int numPB  = sizeof(PB_pin);
const int numLED = sizeof(LED_pin);
byte SW[numSW];     // holds the 0/1 state of the switches
byte SW_old[numSW]; // old state of the switches
byte PB[numPB];     // holds the 0/1 state of the pushbuttons
byte PB_old[numPB]; // old state of the pushbuttons
byte LED_value[numLED], LED_target[numLED]; // current and target PWM value for the LEDs 
byte servo_value, servo_target;
byte encA, encA_old, encB, encB_old;
byte blinking, pushed;
int analog_reading, enc_value, enc_value_old;
unsigned int st = 1;  // states & transitions
unsigned long timetofade, timetoblink, timetomoveservo, enc_rotated_time;
unsigned long SW_time[numSW], PB_time[numPB];

#include <Servo.h>  // Servo functions are now included
Servo servo;        // A servo is declared with the name servo

void setup() {
  pinMode(ENCA_pin, INPUT_PULLUP);
  pinMode(ENCB_pin, INPUT_PULLUP);
  for (byte i=0; i<numPB; i++) pinMode(PB_pin[i],INPUT_PULLUP);
  for (byte i=0; i<numSW; i++) pinMode(SW_pin[i],INPUT_PULLUP);
  pinMode(13, OUTPUT); // on board LED
  digitalWrite(13,LOW);
  // LEDs are not specified as OUTPUT, they are analogWrite() PWM pins
  servo.attach(SERVO_pin,1000,2000); //servo connected (pin, [us] pulse min, [us] pulse max)
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

//////////////////////////////////////////////////////////////////////////
// function: detect pushbuttons state change and return push time 
//////////////////////////////////////////////////////////////////////////
byte read_PB(byte i) {
  PB[i] = digitalRead(PB_pin[i]); // read the push button
  if(PB[i] != PB_old[i]) {        // state has changed
    PB_old[i] = PB[i];
    if(PB[i]==0) PB_time[i] = millis(); // 1 > 0 transition, start timer
    else {                              // 0 > 1 transition, return push time
      int time_pushed = millis() - PB_time[i];
      if     (time_pushed > (int)LONG_PRESS)  return 2;
      else if(time_pushed > (int)SHORT_PRESS) return 1;
      else                                    return 0;
    }
  }
  else return 0;
}

//////////////////////////////////////////////////////////////////////////
// function: read encoder and determine slow or fast rotation
//////////////////////////////////////////////////////////////////////////
int read_encoder() {
  int increment;
  encA = digitalRead(ENCA_pin);
  if((encA_old == 0) && (encA == 1)) { // 0 > 1 transition
    if     ((millis() - enc_rotated_time) > ROTARY_SLOW) increment =  1;
    else if((millis() - enc_rotated_time) > ROTARY_FAST) increment = 10;
    else                                                 increment =  0;
    enc_rotated_time = millis();
    encA_old = encA;
    if(digitalRead(ENCB_pin) == 1) return  increment;
    else                           return -increment;
  }
  encA_old = encA;
  return 0;
}

void loop() {
//////////////////////////////////////////////////////////////////////////
// Read and store the switches, debounced
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<numSW; i++) {
    byte reading = digitalRead(SW_pin[i]);
    if(SW[i]==reading) SW_time[i] = millis();
    else if(millis() > SW_time[i] + (unsigned long)DEBOUNCE) SW[i] = reading;
  } // end for

//////////////////////////////////////////////////////////////////////////
// Example of state transition diagram, st<100: state, st>100: transition
//////////////////////////////////////////////////////////////////////////
	switch(st) {
		case 1: // read pushbuttons (PB) and switches (SW)
      pushed = read_PB(1);
      if(pushed==1) st = 101;  
      if(pushed==2) st = 102;
      pushed = 0;
      if(SW[0]==1) blinking = 1;
      else         blinking = 0;
      if(SW[1]==1) LED_target[fade_LED] = FADE_HIGH;
      else         LED_target[fade_LED] = FADE_LOW;
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
// Blink an LED with different on and off times
//////////////////////////////////////////////////////////////////////////
  if (blinking) {
    if (millis() > timetoblink) {
      if(!LED_value[blink_LED]) { // LED is off, we turn it on now   
        timetoblink = millis() + (unsigned long)BLINK_ON;
        LED_value[blink_LED] = 255;
      }
      else {              // led is on, we turn it off now
        timetoblink = millis() + (unsigned long)BLINK_OFF;
        LED_value[blink_LED] = 0;
      }
    }
  }
  else LED_value[blink_LED] = 0;

//////////////////////////////////////////////////////////////////////////
// Update an LED value if it's time, this creates a fade
//////////////////////////////////////////////////////////////////////////
  if (micros() > timetofade) {
      timetofade = micros() + (unsigned long)FADE_INTERVAL;
      if(LED_value[fade_LED] < LED_target[fade_LED]) LED_value[fade_LED]++;
      if(LED_value[fade_LED] > LED_target[fade_LED]) LED_value[fade_LED]--;
  }

//////////////////////////////////////////////////////////////////////////
// Move the servo a step closer to its target angle if it's time
//////////////////////////////////////////////////////////////////////////
  if (millis() > timetomoveservo) {
    timetomoveservo = millis() + (unsigned long)SERVO_INTERVAL;
    if(servo_value < servo_target) servo_value++;
    if(servo_value > servo_target) servo_value--;
    servo.write(servo_value);
  }

//////////////////////////////////////////////////////////////////////////
// Read the analog input
//////////////////////////////////////////////////////////////////////////
  analog_reading = analogRead(POTM_pin);
// Serial.println(analog_reading);
  
//////////////////////////////////////////////////////////////////////////
// Read the digital encoder
//////////////////////////////////////////////////////////////////////////  
  enc_value = enc_value + read_encoder();
  if(enc_value > 255) enc_value = 255;
  if(enc_value <   0) enc_value =   0;
  if(enc_value != enc_value_old) {
    LED_value(encoder_LED) = enc_value;
    Serial.println(enc_value);
    enc_value_old = enc_value;
  }

//////////////////////////////////////////////////////////////////////////
// Write the PWM values to the LED outputs
//////////////////////////////////////////////////////////////////////////
  for (byte i=0; i<6; i++) analogWrite(LED_pin[i],LED_value[i]);
  

} // end loop
