// Ruud Boer, 2023, July
// Demo box with 6 LEDs, 2 pushbuttons, 2switches, 1 potmeter, 1 encoder, 1 servo

// BEWARE: If servo.h in included, pins 9,10 do NOT work as PWM analogWrite()

// SW0 fades LED_SW0 on/off
// SW1 fades LED_SW1 on/off
// PB0 short/long switches LED_PB0 on/off and toggles servo to MIN/MAX / 90.
// PB1 short/long switches LED_PB1 on/off
// PB2 encoder switches LED_BLINK blinking on/off
// encoder changes interval time for LED_BLINK
// potmeter is dimmer for LED_POTM, value is printed on monitor

#define PWM_LED_LOW      1 // minimum PWM value for LEDs 0-255
#define PWM_LED_HIGH   255 // maximum PWM value for LEDs 0-255
#define SERVO_MIN        5 // [degrees] 0-180
#define SERVO_MAX      175 // [degrees] 0-180
#define FADE_INTERVAL   10 // [ms]
#define SERVO_INTERVAL  30 // [ms] time between servo steps
#define ENC_DEBOUNCE    10 // [ms] encoder debounce
#define ENC_SLOW_FAST  200 // [ms] encoder slow / fast rotation limit
#define ENC_INC_FAST    10 // [ms] encoder in-/decrement with fast rotation
#define BLINK_ON        20 // [ms]
#define BLINK_OFF      500 // [ms] initial, value is changed via the encoder
#define DEBOUNCE        50 // [ms]
#define SHORT_PRESS    200 // [ms]
#define LONG_PRESS    2000 // [ms]

// The pin numbers are for the demo box with the flatcable in the aluminium case
#define SERVO_PIN       12 
#define ENCA_PIN        14 // A0 Encoder A or Data
#define ENCB_PIN        15 // A1 Encoder B or Clock
#define POTM_PIN        17 // A3 Potentiometer Analog In
byte swpin[]     = {8,9};      // SW0, SW1 input pins
byte pbpin[]     = {10,11,16}; // PB0, PB1, encoder input pins
byte digledpin[] = {2,4,7,13}; // digital LED outputs pins, 13 is PCB LED
byte pwmledpin[] = {3,5,6};    // pwm LED output pins NOTE: marked ~ , don't use pins 9,10 with servo.h

////////////////////////
// End of configuration
////////////////////////

#include <Arduino.h>
#include <Servo.h>

Servo servo; // A servo is declared with the name 'servo'
const int numsw     = sizeof(swpin);
const int numpb     = sizeof(pbpin);
const int numdigled = sizeof(digledpin);
const int numpwmled = sizeof(pwmledpin);
byte sw[numsw], swchange[numsw]; // swchange: 0=HL, 1=LH, 2=none
byte pb[numpb], pbstate[numpb], pbstateold[numpb]; // pushbuttons
byte servovalue = 90, servotarget = 90;
byte encA, encAold, encB;
byte blinking;
byte digled[numdigled], pwmledtarget[numpwmled], pwmledvalue[numpwmled];
int analogreading, encvalue = BLINK_OFF, encvalueold = BLINK_OFF;
unsigned int st = 1; // states & transitions
unsigned long timetofade, timetoblink, timetomoveservo, encrottime;
unsigned long swtime[numsw], pbtime[numpb];

void setup() {
  pinMode(ENCA_PIN, INPUT_PULLUP);
  pinMode(ENCB_PIN, INPUT_PULLUP);
  for (byte i=0; i<numpb;  i++) pinMode( pbpin[i], INPUT_PULLUP);
  for (byte i=0; i<numsw;  i++) pinMode( swpin[i], INPUT_PULLUP);
  for (byte i=0; i<numdigled; i++) pinMode(digledpin[i], OUTPUT);
  // pwm_pin[] are not specified as OUTPUT, they are analogWrite() PWM pins
  servo.attach(SERVO_PIN, 1000, 2000); //servo connected (pin, [us] pulse min, [us] pulse max)
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
}

//////////////////////////////////////////////////////////////////////////////
// Function: read and store the states and changes of the switches, debounced
//////////////////////////////////////////////////////////////////////////////
void read_sw() {
  for (byte i=0; i<numsw; i++) {
    byte reading = digitalRead(swpin[i]);
    if(sw[i]==reading) {
      swtime[i] = millis();
      swchange[i] = 2; // 2=no change
    }
    else if(millis() > swtime[i] + (unsigned long)DEBOUNCE) {
      sw[i]        = reading; // store the current state, 0 or 1
      swchange[i]  = reading; // is 0=HL or 1=LH after a change for one loop cycle, is 2 when no change
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Function: detect pushbutton presses and store 0=short-, 1=long-, 2=no press
///////////////////////////////////////////////////////////////////////////////
void read_pb() {
  for (byte i=0; i<numpb; i++) {  
    pbstate[i] = digitalRead(pbpin[i]); // read the push button
    if(pbstate[i] != pbstateold[i]) {   // state has changed
      pbstateold[i] = pbstate[i];
      if(pbstate[i]==0) pbtime[i] = millis(); // HL transition, start timer
      else {                                  // LH transition, store push time
        int timepushed = millis() - pbtime[i];
        if     (timepushed > (int)LONG_PRESS)  pb[i] = 1; // value is only here for one loop cycle
        else if(timepushed > (int)SHORT_PRESS) pb[i] = 0; // value is only here for one loop cycle
        else                                   pb[i] = 2; // not pushed
      }
    }
    else pb[i] = 2; // not pushed
  }
}

//////////////////////////////////////////////////
// Function: detect encoder slow or fast rotation
//////////////////////////////////////////////////
int read_encoder() {
  int increment;
  encA = digitalRead(ENCA_PIN);
  if((encAold == 0) && (encA == 1)) { // 0 > 1 transition
    if     ((millis() - encrottime) > ENC_SLOW_FAST) increment =  1;
    else if((millis() - encrottime) > ENC_DEBOUNCE)  increment = 10;
    else                                             increment =  0;
    encrottime = millis();
    encAold = encA;
    if(digitalRead(ENCB_PIN) == 1) return  increment;
    else                           return -increment;
  }
  encAold = encA;
  return 0;
}

void loop() {
/////////////////////////////////////////////////////////////
// State transition diagram, st<100=state, st>100=transition
/////////////////////////////////////////////////////////////
  switch(st) {
    case 1: // STATE 1:
      read_pb(); // Execute the function to read states of pushbuttons
      read_sw(); // Execute the function to read states of switches
      if(pb[0]==0) st = 101; // PB0 short press
      if(pb[0]==1) st = 102; // PB0 long press
      if(pb[1]==0) st = 103; // PB1 short press
      if(pb[1]==1) st = 104; // PB1 long press
      if(pb[2]==0) st = 109; // PB2 encoder short press
      if(swchange[0]==0) st = 105; // SW0 HL
      if(swchange[0]==1) st = 106; // SW0 LH
      if(swchange[1]==0) st = 107; // SW1 HL
      if(swchange[1]==1) st = 108; // SW1 LH
    break;
    case 101: // TRANS 1: move servo to min/max angle, turn PB0 LED on
      digled[0] = 1;
      if(servotarget==SERVO_MIN) servotarget = SERVO_MAX;
      else                       servotarget = SERVO_MIN;
      Serial.print("101 PB0 short: Servo target = ");
      Serial.println(servotarget);
      st = 1; // return to STATE 1
    break;
    case 102: // TRANS 2: move servo to 90, turn PB0 LED off
      digled[0] = 0;
      servotarget = 90;
      Serial.print("102 PB0  long: Servo target = ");
      Serial.println(servotarget);
      st = 1; // return to STATE 1
    break;
    case 103: // TRANS 3: PB1 short: LED on
      digled[1] = 1;
      Serial.println("103 PB1 short: LED2 ON");
      st = 1; // return to STATE 1
    break;
    case 104: // TRANS 4: LED2 off
      digled[1] = 0;
      Serial.println("104 PB1 long: LED2 OFF");
      st = 1; // return to STATE 1
    break;
    case 105: // TRANS 5: Fade SW0 LED to HIGH
      pwmledtarget[0] = PWM_LED_HIGH;
      Serial.println("105 SW0 HL: LED0 fade HIGH");
      st = 1; // return to STATE 1
    break;
    case 106: // TRANS 6: Fade SW0 LED to LOW
      pwmledtarget[0] = PWM_LED_LOW;
      Serial.println("106 SW0 LH: LED0 fade LOW");
      st = 1; // return to STATE 1
    break;
    case 107: // TRANS 7: Fade SW1 LED to HIGH
      pwmledtarget[1] = PWM_LED_HIGH;
      Serial.println("107 SW1 HL: LED1 fade HIGH");
      st = 1; // return to STATE 1
    break;
    case 108: // TRANS 8: Fade SW1 LED to LOW
      pwmledtarget[1] = PWM_LED_LOW;
      Serial.println("108 SW1 LH: LED1 fade LOW");
      st = 1; // return to STATE 1
    break;
    case 109: // TRANS 9: start/stop blinking 
      blinking = !blinking;
      if(blinking) Serial.println("109 LED5 blink ON");
      else         Serial.println("109 LED5 blink OFF"); 
      st = 1; // return to STATE 1
    break;
  }   // end switch

/////////////////////////////////
// Blink the LED on digledpin[2]
/////////////////////////////////
  if (blinking) {
    if (millis() > timetoblink) {
      if(!digled[2]) { // LED is off, we turn it on now   
        timetoblink = millis() + (unsigned long)BLINK_ON;
        digled[2] = 1;
      }
      else {           // led is on, we turn it off now
        timetoblink = millis() + (unsigned long)encvalue;
        digled[2] = 0;
      }
    }
  }
  else digled[2] = 0;
  digled[3] = digled[2]; // Also blink the on board LED on pin 13

///////////////////////
// Fade pwmled 0 and 1
///////////////////////
  if (millis() > timetofade) {
    timetofade = millis() + (unsigned long)FADE_INTERVAL;
    for (byte i=0; i<numpwmled; i++) {  
      if(pwmledvalue[i] < pwmledtarget[i]) pwmledvalue[i]++;
      if(pwmledvalue[i] > pwmledtarget[i]) pwmledvalue[i]--;
    }
  }

///////////////////////////////////////////////////////////////////
// Move the servo a step closer to the target angle when it's time
///////////////////////////////////////////////////////////////////
  if (millis() > timetomoveservo) {
    timetomoveservo = millis() + (unsigned long)SERVO_INTERVAL;
    if(servovalue < servotarget) servovalue++;
    if(servovalue > servotarget) servovalue--;
    servo.write(servovalue);
  }

//////////////////////////////////////////
// Read the analog input and set pwmled 2
//////////////////////////////////////////
  analogreading = analogRead(POTM_PIN);
  pwmledvalue[2] = analogreading / 4;
  
////////////////////////////////////////////////////////////
// Read the digital encoder and update encvalue accordingly
//////////////////////////////////////////////////////////// 
  encvalue = encvalue + read_encoder();
  if(encvalue > 1000) encvalue = 1000;
  if(encvalue <   20) encvalue =   20;
  if(encvalue != encvalueold) {
    encvalueold = encvalue;
    Serial.println(encvalue);
  }

////////////////////////////////////////////////////
// Write the PWM- and digital values to the led outputs
////////////////////////////////////////////////////
  for (byte i=0; i<numpwmled; i++) analogWrite(pwmledpin[i], pwmledvalue[i]);
  for (byte i=0; i<numdigled; i++) digitalWrite(digledpin[i], digled[i]);

} // end loop
