#define NUMSERVOS   2 // Enter the number of servos here
#define SERVOSPEED 30 // [ms] between servo updates, lower is faster

// GO TO setup() TO CONFIGURE DCC ADDRESSES, PIN NUMBERS, SERVO ANGLES

#include <DCC_Decoder.h>
#include <Servo.h>

unsigned long timetomove;

typedef struct DCCAccessoryData {
  int   address;   // User Configurable DCC address
  byte  dccstate;  // The DCC state of this accessory: 1=on, 0=off
  byte  angle;     // Current angle of servo
  byte  setpoint;  // Setpoint angle of servo
  byte  offangle;  // User Configurable servo angle for DCC state = 0
  byte  onangle;   // User Configurable servo angle for DCC state = 1
  Servo servo;
};
DCCAccessoryData servo[NUMSERVOS];

void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data) {
  address -= 1;
  address *= 4;
  address += 1;
  address += (data & 0x06) >> 1;
  // address = address - 4 // uncomment this line for Roco Maus or Z21
  boolean enable = (data & 0x01) ? 1 : 0;
  for (int i=0; i<NUMSERVOS; i++) {
    if (address == servo[i].address) {
      if (enable) servo[i].dccstate = 1;
      else        servo[i].dccstate = 0;
    }
  }
}

void setup() { 
// CONFIGURATION OF SERVOS
// Copy & Paste as many times as you have servos 
// The amount must be same as NUMSERVOS in line 1!
// Don't forget to increment the array index 0,1,2,...
  servo[0].address   =  100 ; // DCC address
  servo[0].servo.attach(  3); // Arduino servo pin
  servo[0].offangle  =   70 ; // Servo angle for DCC state = 0
  servo[0].onangle   =  110 ; // Servo angle for DCC state = 1

  servo[1].address   =  101 ; // DCC address
  servo[1].servo.attach(  4); // Arduino servo pin
  servo[1].offangle  =   70 ; // Servo angle for DCC state = 0
  servo[1].onangle   =  110 ; // Servo angle for DCC state = 1


  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC.SetupDecoder( 0x00, 0x00, 0 );

  for(byte i=0; i<NUMSERVOS; i++) {
    servo[i].angle = servo[i].offangle;
    servo[i].servo.write(servo[i].angle);
    delay(1000); // wait 1 second before activating the next servo
  }
}

void loop() {
  static int n = 0;
  DCC.loop(); // DCC library
  if( ++n >= NUMSERVOS ) n = 0; // Next address to test
	
  if (servo[n].dccstate == 1) servo[n].setpoint = servo[n].onangle;
  else                        servo[n].setpoint = servo[n].offangle;

// Move the servos when it is 'timetomove'
  if (millis() > timetomove) {
    timetomove = millis() + (unsigned long)SERVOSPEED;
    for (byte i=0; i<NUMSERVOS; i++) {
      if (servo[i].angle < servo[i].setpoint) servo[i].angle++;
      if (servo[i].angle > servo[i].setpoint) servo[i].angle--;
      servo[i].servo.write(servo[i].angle);
    } // end for
  }   // end if
}     // end loop
