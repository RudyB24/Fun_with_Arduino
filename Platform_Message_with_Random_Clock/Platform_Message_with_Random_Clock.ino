// OLED Model railway Station Platform Display - Rudy B, August 2019
// 6 different messages can be shown, based on 6 inputs
// OLED SSD1306 - I2C wires: SDA or A4, SCL or A5

#define MSG1_PIN   2
#define MSG2_PIN   3
#define MSG3_PIN   4
#define MSG4_PIN   5
#define MSG5_PIN   6
#define MSG6_PIN   7
#define COLOR_PIN  8
#define TMIN       5 // departure time of next train, minimum of random time
#define TMAX      13 // departure time of next train, maximum of random time

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// setup oled
Adafruit_SSD1306 oled(4);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
byte hour, minute, second; 
byte bg, fg, msgnr, msgnr_old, msgflag, msghour, msgminute;
unsigned long time_to_update;
float angle;

void calc_msg_time() {
  msgminute = msgminute + random(TMIN, TMAX);
  if (msgminute > 59) {
    msghour = (msghour + 1) % 24;
    msgminute = msgminute - 60;
  }
}

void print_msg_time() {
  oled.setTextColor(fg);
  oled.setTextSize(1);
  oled.setCursor(40,2);
  if(msghour < 10) oled.print(" ");
  oled.print(msghour);
  oled.print(":");
  if(msgminute < 10) oled.print("0");
  oled.print(msgminute);
}

void setup() {
  pinMode(MSG1_PIN,  INPUT_PULLUP);
  pinMode(MSG2_PIN,  INPUT_PULLUP);
  pinMode(MSG3_PIN,  INPUT_PULLUP);
  pinMode(MSG4_PIN,  INPUT_PULLUP);
  pinMode(MSG5_PIN,  INPUT_PULLUP);
  pinMode(MSG6_PIN,  INPUT_PULLUP);
  pinMode(COLOR_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C

// generate random startup time
  randomSeed(analogRead(A0));
  hour   = random(7, 20);
  minute = random(0, 60);
  msgminute = minute;
  msghour = hour;
}

void loop() {

// determine background- and foreground color based on COLOR_PIN
  if(digitalRead(COLOR_PIN) == LOW) {bg = WHITE; fg = BLACK;}
  else {bg = BLACK; fg = WHITE;}

// update time
  if(millis() > time_to_update) {
    time_to_update = millis() + 1000UL;
    second = second + 1;
    if(second == 60) {
      second = 0;
      minute = minute + 1;
    }
    if (minute == 60) {
      minute = 0;
      hour = (hour + 1) % 24;
    }
  }

// clear oled  
  oled.clearDisplay();
  oled.fillRect(0,0,127,31,bg);

// draw clock ticks
  for(int z=0; z<360;z=z+30){
    angle = (float)z / 57.3;
    int x1=(16+(sin(angle)*15));
    int y1=(15-(cos(angle)*15));
    int x2=(16+(sin(angle)*(12)));
    int y2=(15-(cos(angle)*(12)));
    oled.drawLine(x1,y1,x2,y2,fg);
  }
// draw clock second
  angle=((float)second * 6 / 57.3) ; //Convert degrees to radians  
  int x2=(16+(sin(angle)*(14)));
  int y2=(15-(cos(angle)*(14)));
  oled.drawLine(16,15,x2,y2,fg);

// draw clock minute
  angle=((float)minute * 6 / 57.3) ; //Convert degrees to radians  
  x2=(16+(sin(angle)*(12)));
  y2=(15-(cos(angle)*(12)));
  oled.drawLine(16,15,x2,y2,fg);

// draw clock hour
  angle=((float)hour * 30 + (float)minute / 2) / 57.3 ; //Convert degrees to radians  
  x2=(16+(sin(angle)*(10)));
  y2=(15-(cos(angle)*(10)));
  oled.drawLine(16,15,x2,y2,fg);

/*
// platform number
  oled.fillRect(107, 5, 19, 22, fg);
  oled.setTextSize(2);
  oled.setTextColor(bg);
  oled.setCursor(111,9);
  oled.print("3"); 
*/

// display time and messages
  if(!digitalRead(MSG1_PIN)) msgnr = 1;
  if(!digitalRead(MSG2_PIN)) msgnr = 2;
  if(!digitalRead(MSG3_PIN)) msgnr = 3;
  if(!digitalRead(MSG4_PIN)) msgnr = 4;
  if(!digitalRead(MSG5_PIN)) msgnr = 5;
  if(!digitalRead(MSG6_PIN)) msgnr = 6;

  if(msgnr != msgnr_old) {
    calc_msg_time();
    msgnr_old = msgnr;
  }
  
  print_msg_time();

  switch (msgnr) {
    case 1:
      oled.setCursor(72,2);  oled.print("Intercity");    // max 10 characters
      oled.setCursor(40,12); oled.print("Eindhoven");    // max 14 characters
      oled.setCursor(40,22); oled.print("via Strijp-S"); // max 14 characters
    break;

    case 2:
      oled.setCursor(72,2);  oled.print("Sprinter");
      oled.setCursor(40,12); oled.print("sHertogenbosch");
      oled.setCursor(40,22); oled.print("via Boxtel");
    break;

    case 3:
      oled.setCursor(72,2);  oled.print("Stoptrein");
      oled.setCursor(40,12); oled.print("Maliebaan");
      oled.setCursor(40,22); oled.print("via Eindhoven");
    break;

      case 4:
      oled.setCursor(72,2);  oled.print("Intercity");
      oled.setCursor(40,12); oled.print("Venlo");    
      oled.setCursor(40,22); oled.print("via Deurne");
    break;

    case 5:
      oled.setCursor(72,2);  oled.print("Intercity");
      oled.setCursor(40,12); oled.print("Maastricht");
      oled.setCursor(40,22); oled.print("via Sittard");
    break;

    case 6:
      oled.setCursor(72,2);  oled.print("Intercity");
      oled.setCursor(40,12); oled.print("Heerlen");  
      oled.setCursor(40,22); oled.print("via Weert");
    break;
  }
// refresh screen
  oled.display();
}