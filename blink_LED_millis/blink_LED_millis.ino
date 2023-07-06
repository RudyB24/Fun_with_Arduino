// Rudy B - November 2018
// Blink a LED using millis() iso of delay()
// NOTE: ATTiny millis() may run twice as fast, 2000 millis is 1s.

#define LED_PIN    1 // pin number of LED. PCB: UNO=13, Nano=13, ATTiny=1
#define ONTIME    20 // [ms]
#define OFFTIME 1980 // [ms]

byte led_state; // 0=OFF, 1=ON
unsigned long time_to_change;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (millis() > time_to_change) {
    led_state = !led_state;
    if (led_state)
      time_to_change = millis() + (unsigned long)ONTIME;
    else
      time_to_change = millis() + (unsigned long)OFFTIME;
    digitalWrite(LED_PIN, led_state);
  }
}
