// Ruud Boer - May 2022
// Tune RGBW LED lighting and display the values
// Inputs are active when 0, LOW, GND
// 4 inputs are used to tune RGBW values
// 1 input is used to tune up or down
// 1 input is used to reset all values to 128

//--------------------------------------------------------------------------------------------------
// Data to be filled in:
//--------------------------------------------------------------------------------------------------

#define FILTERTIME 100 // [ms] Switch inputs must be stable FILTERTIME ms before being accepted
//                         R   G   B   W
byte input_pin[]      = {  2,  3,  4,  5, 7, 8}; // 7:up/down, 8:reset to 128
byte LED_output_pin[] = {  6,  9, 10, 11};       // Must be PWM outputs marked with ~
int step_interval     =   100; // [ms] Higher number = slower tuning
int print_interval    =  1000; // [ms] Time between print RGBW values

//--------------------------------------------------------------------------------------------------
// End of configuration. Don't change anything below this line
//--------------------------------------------------------------------------------------------------

const int numinputs = sizeof(input_pin); // How many inputs are in use with this Arduino
byte sw[numinputs], i;
byte R=128, G=128, B=128, W=128;
unsigned long time_for_led_step, time_to_print, sw_time[numinputs];

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, 0);
  for (i = 0; i < numinputs; i++) {
    pinMode(input_pin[i], INPUT_PULLUP);
    sw[i] = 1;
  } 
  Serial.begin(9600);
  Serial.println(numinputs);
  Serial.println("Ready");
}

void loop() {
  
  for (byte i=0; i<numinputs; i++) { // Read the switches with anti interference FILTERTIME
    byte reading = digitalRead(input_pin[i]);
    if(reading==sw[i]) {
      sw_time[i] = millis();
    }
    else if(millis() > sw_time[i] + (unsigned long)FILTERTIME) {
      sw[i] = reading;
    }
  }
  
  if(!sw[5]) {
    R = 128; G = 128; B = 128; W = 128; 
  }

  if(millis() > time_for_led_step) {
    time_for_led_step = millis() + (unsigned long)step_interval;
    if(!sw[0] && sw[4]) R++; if(!sw[0] && !sw[4]) R--;
    if(!sw[1] && sw[4]) G++; if(!sw[1] && !sw[4]) G--;
    if(!sw[2] && sw[4]) B++; if(!sw[2] && !sw[4]) B--;
    if(!sw[3] && sw[4]) W++; if(!sw[3] && !sw[4]) W--;
    analogWrite(LED_output_pin[0], R);
    analogWrite(LED_output_pin[1], G);
    analogWrite(LED_output_pin[2], B);
    analogWrite(LED_output_pin[3], W);
  }

  if(millis() > time_to_print) {
    time_to_print = millis() + (unsigned long)print_interval;
    Serial.print(R); Serial.print(" ");
    Serial.print(G); Serial.print(" ");
    Serial.print(B); Serial.print(" ");
    Serial.print(W); Serial.println(" ");
  }
}
