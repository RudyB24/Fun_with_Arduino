// Ruud Boer
// Original version August 2019
// Revised version September 2021
// Two optical sensors are used, spaced SENSOR_DISTANCE [um] apart
// [us] timer starts when one of the sensor inputs goes LOW
// [us] timer stops when the other sensor goes LOW
// m_per_s = distance / (stop_time - start_time)
// km_per_hr = 3.6 * m_per_s, mph = 2.23694 * m_per_s;
// REMARK: connect OLED display VDD to Arduino 3.3V (may depend on OLED type)
//
// SCALE: HO 1/87. To change scale connect pin to GND:
//        A0=O 1/45, A1=OO 1/76, A2=TT 1/120, A3=N 1/160
//
// UNITS: km/hr. To change units to MPH connect pin 8 to GND

#define SENSOR_DISTANCE  200000 // [um] measured distance between the two IR beams
#define INIT_TRAIN_LENGTH    20 // [cm] New measurement only starts when train fully passed
                                // Loco Length can be changed via keyboard input
#define SENSOR_L_PIN 6
#define SENSOR_R_PIN 7

#include <Wire.h> //I2C library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(4); // define OLED display()

byte l_r, state;
byte scale, scale_old;
byte units, units_old=99; // 0=MPH, 1=km/hr, 99=startup dummy
int train_length, serialread;
unsigned long start_us, stop_us, measured_us, waittime;
float m_per_s;
float km_per_hr;
float mi_per_hr;

void write_to_display() {
  display.clearDisplay();
	display.setTextColor(WHITE,BLACK);
	if (!units) { // units = 0: MPH
	  if ((mi_per_hr)<10) display.setCursor(30,0); else display.setCursor(6,0);
		display.setTextSize(4);
		display.print(mi_per_hr);
		display.setCursor(59,0);
		display.setTextSize(1);
		display.print("MI");
		if ((km_per_hr)<10) display.setCursor(6,48); else display.setCursor(0,48);
		display.setTextSize(1);
		display.print(km_per_hr);
		display.setCursor(42,48);
		display.print("km/h");
	}
	else { // units = 1: km/hr
		if ((km_per_hr)<10) display.setCursor(30,0); else display.setCursor(6,0);
		display.setTextSize(4);
		display.print(km_per_hr);
		display.setCursor(59,0);
		display.setTextSize(1);
		display.print("KM");
		if ((mi_per_hr)<10) display.setCursor(6,48); else display.setCursor(0,48);
		display.setTextSize(1);
		display.print(mi_per_hr);
		display.setCursor(36,48);
		display.print(" MPH");
	}
	if ((m_per_s)<10) display.setCursor(6,57); else display.setCursor(0,57);
  display.print(m_per_s);
	display.setCursor(36,57);
  display.print(" m/s");
	display.setCursor(79,48);
  display.print(int((measured_us+500)/1000));
	display.setCursor(115,48);
  display.print("ms");
	display.setCursor(79,57);
  display.print("1/");
  display.print(scale);
  display.setCursor(115,57);
  if (scale ==  45) display.print(" O");
  if (scale ==  76) display.print("OO");
  if (scale ==  87) display.print("HO");
  if (scale == 120) display.print("TT");
  if (scale == 160) display.print(" N");

	display.drawRect(13,36,102,5,WHITE);
	display.fillRect(14,37,100,3,BLACK);
  if(units) display.fillRect(14,37,int(km_per_hr+0.5),3,WHITE);  
	else      display.fillRect(14,37,int(mi_per_hr+0.5),3,WHITE);  
	display.display();
}

void read_write_scale() {
  if (Serial.available()) {
    serialread = Serial.parseInt();
    if(serialread) train_length = serialread;
    Serial.print(F("Train length set to: "));
    Serial.print(train_length);
    Serial.println(F(" cm"));
    Serial.println(F("Waiting for train ..."));
    Serial.println();
  }
	scale = 87;                        // scale HO
	if (!digitalRead(14)) scale = 45;  // scale O
	if (!digitalRead(15)) scale = 76;  // scale OO
	if (!digitalRead(16)) scale = 120; // scale TT
	if (!digitalRead(17)) scale = 160; // scale N
  if(scale != scale_old ) {
    scale_old = scale;
    Serial.print(F("Scale set to: 1/"));
    Serial.println(scale);
    write_to_display();
  }
	units = digitalRead(8); // 0=MPH, 1=km/hr
  if(units != units_old ) {
    units_old = units;
    Serial.print(F("Units set to: "));
    if(units) Serial.println(F("km/hr"));
    else      Serial.println(F("MPH"));
    write_to_display();
    Serial.println(F("Waiting for train..."));
  }
}

void setup() {
  pinMode(13, OUTPUT); // on board LED
	pinMode(SENSOR_L_PIN,INPUT_PULLUP);
	pinMode(SENSOR_R_PIN,INPUT_PULLUP);
	pinMode( 8,INPUT_PULLUP); 
	pinMode(14,INPUT_PULLUP);
	pinMode(15,INPUT_PULLUP);
	pinMode(16,INPUT_PULLUP);
	pinMode(17,INPUT_PULLUP);
	pinMode(18,INPUT_PULLUP);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize OLED display
  train_length = INIT_TRAIN_LENGTH; // [cm]
  Serial.println();
  Serial.print(F("Train length is set to "));
  Serial.print(train_length);
  Serial.println(F(" cm "));
  Serial.println(F("For longer trains change this by typing"));
  Serial.println(F("in the input field above and hit ENTER"));
//  Serial.println(F("For auto measurement with Traincontroller:"));
//  Serial.println(F("1: Open TC Speed Profiling Tab"));
//  Serial.println(F("2: Start AutoHotKey"));
  Serial.println();
  read_write_scale();
  write_to_display();
  display.fillRect(0,36,7,5,WHITE);
  display.fillRect(120,36,7,5,WHITE);
  display.display();
	digitalWrite(13, HIGH); // LED on means ready for measurement
}

void loop() {
  read_write_scale();

  switch (state) {
		case 0: // initial state, ready to start measuring
			if (!digitalRead(SENSOR_L_PIN)) {start_us = micros(); l_r = 0; state = 1;}
			if (!digitalRead(SENSOR_R_PIN)) {start_us = micros(); l_r = 1; state = 1;}
		break;

		case 1: // wait for the other sensor to be triggered
			digitalWrite(13, LOW);
			display.fillRect(0,48,127,15,BLACK);
			display.setCursor(3,48);
			display.setTextSize(2);
			if (!l_r) {
        Serial.println(F("L >>> R"));
				display.print("L >>>>>> R");
        display.fillRect(0,36,7,5,BLACK);
				display.display();
				while (digitalRead(SENSOR_R_PIN)) {} // loop here until sensor R is triggered
			}
			else {
        Serial.println(F("L <<< R"));
				display.print("L <<<<<< R");
        display.fillRect(120,36,7,5,BLACK);
				display.display();
				while (digitalRead(SENSOR_L_PIN)) {} // loop here until sensor L is triggered
			}
			stop_us = micros();
			state = 2;
		break;

		case 2: // calculate and show speed values
			measured_us = stop_us - start_us - 10UL; // -10 to compensate for code delay
			m_per_s = float(scale) * float(SENSOR_DISTANCE) / float(measured_us);
			km_per_hr = 3.6 * m_per_s;
			mi_per_hr = 2.23694 * m_per_s;
			write_to_display();
      if (!units) {Serial.print(mi_per_hr); Serial.println(F(" MPH"));}
      else        {Serial.print(km_per_hr); Serial.println(F(" km/hr"));}
      Serial.println();
      waittime = (int)(10UL * measured_us * (unsigned long)train_length / (unsigned long)SENSOR_DISTANCE + 1000UL); // [ms]
			// measured_us is needed for a move of SENSOR_DISTANCE um
			// measured_us * train_length / SENSOR_DISTANCE) time is needed for train_length to pass
      Serial.print(F("Wait "));
      Serial.print(waittime);
      Serial.println(F(" ms"));
      delay(waittime);
      digitalWrite(13, HIGH); //Next measurement can start when LED is on
      display.fillRect(0,36,7,5,WHITE);
      display.fillRect(120,36,7,5,WHITE);
      display.display();
      Serial.println(F("Waiting for train..."));
      state = 0;
		break;
	}
}
