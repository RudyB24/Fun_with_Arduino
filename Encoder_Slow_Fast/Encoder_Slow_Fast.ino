// Ruud Boer, March 2022
// A digital encoder is read via a function and it returns:
// - 10 if the pulses came faster than ENCODER_SLOW_FAST ms
// - 1 if the pulses came slower than ENCODER_SLOW_FAST ms
// - 0 if the pulses came faster than ENC_DEBOUNCE ms

#define ENCA_PIN        14 // A0 Encoder A or Data
#define ENCB_PIN        15 // A1 Encoder B or Clock
#define ENC_DEBOUNCE    10 // [ms] rotary debounce time
#define ENC_SLOW_FAST  200 // [ms] rotary slow / fast rotation limit

byte encA, encA_old, encB;
int enc_value, enc_value_old;
unsigned long enc_rot_time;

void setup() {
  pinMode(ENCA_PIN, INPUT_PULLUP);
  pinMode(ENCB_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");
} // End setup()

//////////////////////////////////////////////////////////////////////////
// Function: determine encoder slow or fast rotation
//////////////////////////////////////////////////////////////////////////
int read_encoder() {
  int increment;
  encA = digitalRead(ENCA_PIN);
  if((encA_old == 0) && (encA == 1)) { // 0 > 1 transition
    if     ((millis() - enc_rot_time) > ENC_SLOW_FAST) increment =  1;
    else if((millis() - enc_rot_time) > ENC_DEBOUNCE)  increment = 10;
    else                                               increment =  0;
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
// Read the digital encoder and change LED_ENCODER brightness accordingly
//////////////////////////////////////////////////////////////////////////  
  enc_value = enc_value + read_encoder();
  if(enc_value > 255) enc_value = 255;
  if(enc_value <   0) enc_value =   0;
  if(enc_value != enc_value_old) {
    Serial.println(enc_value);
    enc_value_old = enc_value;
  }

} // End loop()
