#define GATE_SPEED          120 // [ms] lower number is higher servo speed
#define BLINK_SPEED         400 // [ms] smaller number is faster blinking
#define GATE_DELAY         2000 // [ms] time between start blinking and gate closing
#define END_OF_TRAIN_DELAY 2000 // [ms] time to wait before deciding this was the end of the train
#define GATE_OPEN_ANGLE_1    90
#define GATE_CLOSED_ANGLE_1  10
#define GATE_OPEN_ANGLE_2    90
#define GATE_CLOSED_ANGLE_2  10
#define SERVO1_PIN            7
#define SERVO2_PIN           12
#define LED1_PIN              2
#define LED2_PIN              3
#define NUM_SENSORS           4 // two sensors per track, one left and one right of the gate
byte sensor_pin[NUM_SENSORS]  = {8,9,10,11}; // sensor pin numbers

byte state = 1;
byte train_counter, n;
byte led1, led2, blink_enabled;
byte    angle1 = GATE_OPEN_ANGLE_1;
byte setpoint1 = GATE_OPEN_ANGLE_1;
byte    angle2 = GATE_OPEN_ANGLE_2;
byte setpoint2 = GATE_OPEN_ANGLE_2;
byte sensor_state[NUM_SENSORS]; // 0 idle, 1 detect arrival, 2 detect departure, 3 detect end of train
byte end_of_train[NUM_SENSORS]; // 0 idle, 1 end of train detected
unsigned long time_to_blink;
unsigned long time_to_close_gate;
unsigned long time_for_servo_step;
unsigned long time_end_of_train[NUM_SENSORS];

#include <Servo.h>
Servo gate_servo1;
Servo gate_servo2;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  for (byte i = 0; i < NUM_SENSORS; i++) pinMode(sensor_pin[i], INPUT_PULLUP);
  gate_servo1.attach(SERVO1_PIN);
  gate_servo1.write(angle1);
  gate_servo2.attach(SERVO2_PIN);
  gate_servo2.write(angle2);
  delay(1000);
  gate_servo1.detach();
  gate_servo2.detach();
  Serial.begin(9600);
  Serial.println("Railway Crossing Control Ready");
  Serial.println();
  Serial.println("Waiting for train");
  for (byte i = 0; i < NUM_SENSORS; i++) sensor_state[i] = 1; // enable sensors for train detection
}

void loop() {

  for (byte i = 0; i < NUM_SENSORS; i++) {
    if(sensor_state[i] == 1) { // detect arrival of new train
      if(!digitalRead(sensor_pin[i])) { // train detected
        train_counter++;
        sensor_state[i] = 0;
        if(i%2) n = i - 1; else n = i + 1;
        sensor_state[n] = 2; // buddy sensor departure detection enabled
        Serial.print("Arrival:   ");
        Serial.println(i);
        Serial.print("Trains:    ");
        Serial.println(train_counter);
      }
    }
    else if(sensor_state[i] > 1) {
      if(!digitalRead(sensor_pin[i])) { // departure detected
        time_end_of_train[i] = millis() + (unsigned long)END_OF_TRAIN_DELAY;
        if(i%2) n = i - 1; else n = i + 1;
        sensor_state[n] = 1; // buddy sensor enabled again
        if(sensor_state[i] == 2) {
          Serial.print("Departure: ");
          Serial.println(i);
        }
        sensor_state[i] = 3;
      }
      if(sensor_state[i] == 3) // decide if end of train has passed based on a timer
        if(millis() > time_end_of_train[i]) end_of_train[i] = 1; 
      if(end_of_train[i]) { // this takes care train_counter-- is executed only once
        train_counter--;
        end_of_train[i] = 0;
        sensor_state[i] = 1;
        Serial.print("Trains:    ");
        Serial.println(train_counter);
      }
    }
  }

  switch (state) {
    case 1: // Gates open. Not blinking. Waiting for train.
      if(train_counter) { // A train is detected.
      Serial.println("Binking started");
      blink_enabled = 1;
      time_to_close_gate = millis() + (unsigned long)GATE_DELAY;
      state = 2;
      }
    break;
      
    case 2: // Blinking. Wait until it's time to close the gates
      if (millis() > time_to_close_gate) { // Gate delay time has passed
        Serial.println("Gate1 closing");
        gate_servo1.attach(SERVO1_PIN);
        setpoint1 = GATE_CLOSED_ANGLE_1;
        state = 3;
      }
    break;

    case 3: // Gate1 closing. Blinking.
      if(angle1 == setpoint1) { // Gate1 closed
        gate_servo1.detach();
        Serial.println("Gate1 closed");
        Serial.println("Gate2 closing");
        gate_servo2.attach(SERVO2_PIN);
        setpoint2 = GATE_CLOSED_ANGLE_2;
        state = 4;
      }
    break;

    case 4: // Gate2 closing. Blinking.
      if(angle2 == setpoint2) { // Gate2 is fully closed
        gate_servo2.detach();
        Serial.println("Gate2 closed");
        state = 5;
      }
    break;
    
    case 5: // Gate2 closed. Blinking. Wait until all trains are gone.
      if(!train_counter) { // End of last train detected
        Serial.println("Gate2 opening");
        gate_servo2.attach(SERVO2_PIN);
        setpoint2 = GATE_OPEN_ANGLE_2;        
      state = 6;
      }
    break;
    
    case 6: // Gate2 opening. Blinking.
      if(train_counter) state = 3; // Another train is coming
      else if(angle2 == setpoint2) { // Gate2 open
        gate_servo2.detach();
        Serial.println("Gate2 open");
        Serial.println("Gate1 opening");
        gate_servo1.attach(SERVO1_PIN);
        setpoint1 = GATE_OPEN_ANGLE_1;
        state = 7;
      }
    break;
    
    case 7: // Gate1 opening. Blinking.
      if(train_counter) state = 2; // Another train is coming
      else if(angle1 == setpoint1) { // Gate1 open
        gate_servo1.detach();
        Serial.println("Gate1 open");
        Serial.println("Stop blinking");
        blink_enabled = 0; // Stop blinking
        led1 = 0;
        led2 = 0;
        Serial.println();
        Serial.println("Waiting for train");
        state = 1;
      }
    break;
  }

  if (millis() > time_for_servo_step) {
    time_for_servo_step = millis() + (unsigned long)GATE_SPEED;
    if (angle1 < setpoint1) angle1++;
    if (angle1 > setpoint1) angle1--;
    if (angle2 < setpoint2) angle2++;
    if (angle2 > setpoint2) angle2--;
    gate_servo1.write(angle1);
    gate_servo2.write(angle2);
  }
    
  if(blink_enabled == 1) {
    if(millis() > time_to_blink) {
      time_to_blink = millis() + (unsigned long)BLINK_SPEED;
      led1 = !led1;
      led2 = !led1;
    }
  }

  digitalWrite(LED1_PIN, led1);
  digitalWrite(LED2_PIN, led2);
}
