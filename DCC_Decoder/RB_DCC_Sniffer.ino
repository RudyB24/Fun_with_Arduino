// DCC packet capture: Robin McKay     March   2014
// DCC packet analyze: Ruud Boer       October 2015
// Improvements      : Jürgen Winkler  March   2016
// Further changes   : Ruud Boer       August  2019
//
// Connect the DCC signal (0V-5V) to pin 2 (uses HW interrupt)

#define NOT_SHOWACC_PIN   3 // Make LOW to NOT show DCC Accessory commands
#define NOT_SHOWLOC_PIN   4 // Make LOW to NOT show DCC Locomotive commands
#define SHOWVT_PIN        5
#define LED_ON_TIME     100 // [ms] indicator LED that DCC Accessory packet was received
#define PACKETBUFFERSIZE  8 // DCC packets buffer size
#define REFRESHTIME       4 // [s] Time between DCC packets buffer refreshes in
#define DCCBITTIMERCOUNT 18 // 18x4=72 [us] to determine between DCC 1 (50us) OR DCC 0 (100us)

// OLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLED
//#include "SSD1306Ascii.h"
//#include "SSD1306AsciiAvrI2c.h"
//#define I2C_ADDRESS 0x3C
//SSD1306AsciiAvrI2c oled;

boolean packetEnd;
boolean preambleFound;
const byte bitBufSize = 50; // number of slots for bits
volatile byte bitBuffer[bitBufSize]; 
volatile byte bitBuffHead = 1;
volatile byte bitBuffTail = 0;
byte checksum;
byte pktByteCount;
byte packetBytesCount;
byte preambleOneCount;
byte dccPacket[6]; // buffer to hold a DCC packet
byte DCCcommand[PACKETBUFFERSIZE][7]; // stores DCC packets: [n][0]=pktByteCount, [n][1-6]=DCC bytes
byte instrByte1;
byte decoderType; //0=Loc, 1=Acc
byte bufferCounter;
byte isSameByte, isSameDCCPacket;
byte showLoc, showAcc;
byte speed;
unsigned int decoderAddress;
unsigned int packetNew;
unsigned long packetBuffer[PACKETBUFFERSIZE];
unsigned long timeToRefresh, LED_off_time;

void getPacket() {
  preambleFound = false;
  packetEnd = false;
  packetBytesCount = 0;
  preambleOneCount = 0;
  while (! packetEnd) {
    if (preambleFound) getNextByte();
    else checkForPreamble();
  }
}

void checkForPreamble() {
   byte nextBit = getBit();
   if (nextBit == 1) preambleOneCount++;
   if (preambleOneCount < 10 && nextBit == 0) preambleOneCount = 0;
   if (preambleOneCount >= 10 && nextBit == 0) preambleFound = true;
}

void getNextByte() {
  byte newByte = 0;
  for (byte i=0; i<8; i++) newByte = (newByte << 1) + getBit();
  packetBytesCount ++;  
  dccPacket[packetBytesCount] = newByte;
  dccPacket[0] = packetBytesCount;
  if (getBit() == 1) packetEnd = true;
}

byte getBit() {
  // gets the next bit from the bitBuffer
  // if the buffer is empty it will wait indefinitely for bits to arrive
  byte nbs = bitBuffHead;
  while (nbs == bitBuffHead) byte nbs = nextBitSlot(bitBuffTail); //Buffer empty
  bitBuffTail = nbs;
  return (bitBuffer[bitBuffTail]);
}

void beginBitDetection() {
  TCCR0A &= B11111100;
  attachInterrupt(0, startTimer, RISING);
}

void startTimer() {
  OCR0B = TCNT0 + DCCBITTIMERCOUNT;
  TIMSK0 |= B00000100;
  TIFR0  |= B00000100;
}

ISR(TIMER0_COMPB_vect) {
  byte bitFound = ! ((PIND & B00000100) >> 2); 
  TIMSK0 &= B11111011;
  byte nbs = nextBitSlot(bitBuffHead);
  if (nbs == bitBuffTail) return;
  else {
    bitBuffHead = nbs;
    bitBuffer[bitBuffHead] = bitFound;
  }
}

byte nextBitSlot(byte slot) {
  slot ++;
  if (slot >= bitBufSize) slot = 0;
  return(slot);
}

void displayCV(byte mode, short cvAddress, byte cvValue) {
  switch(mode) {
    case 1: Serial.print("Verify "); break;
    case 2: Serial.print("Bit "); break;
    case 3: Serial.print("Write "); break;
    case 10: Serial.print("Bit write "); break;
    case 11: Serial.print("Bit verify "); break;
    default: Serial.print("reserved "); break;
  }
  Serial.print("Addr ");
  Serial.print(cvAddress);
  if (mode==2 || mode==10 || mode==11) {  // bit opertion
    Serial.print(" Bit ");
    Serial.print(dccPacket[pktByteCount-1]&B00000111);
    Serial.print("=");
    Serial.print((dccPacket[pktByteCount-1]&B00001000)>>3);
  } else {  // byte operation
    Serial.print(" Value ");
    Serial.print(cvValue);
  }
}

void checkUserInput() { // Check if to show Acc, show Loc, show Voltage, show Time
  showAcc = digitalRead(NOT_SHOWACC_PIN);
  showLoc = digitalRead(NOT_SHOWLOC_PIN);
//  showAcc = 1;
//  showLoc = 1;
}

void printWithLeadingZeros4(int value) {
  if(value<1000) Serial.print(0);
  if(value< 100) Serial.print(0);
  if(value<  10) Serial.print(0);
  Serial.print(value);
}

void printWithLeadingZeros3(int value) {
  if(value< 100) Serial.print(0);
  if(value<  10) Serial.print(0);
  Serial.print(value);
}

void printLocFunctions(byte value, byte functions) {
  for(byte i=0; i<functions; i++) {
    Serial.print(bitRead(value,i));
  }
}

void evaluateAndPrint() {
/*
  for(byte i=0; i<=pktByteCount; i++) { // Print the DCC bytes, for test purposes
    Serial.print(dccPacket[i]);
    Serial.print(" ");
  }
*/
  if (!bitRead(dccPacket[1],7)) { //bit7=0 -> Loc Decoder Short Address
    decoderAddress = dccPacket[1];
    instrByte1 = dccPacket[2];
    decoderType = 0;
  }
  else {
    if (bitRead(dccPacket[1],6)) { //bit7=1 AND bit6=1 -> Loc Decoder Long Address
      decoderAddress = 256 * (dccPacket[1] & B00000111) + dccPacket[2];
      instrByte1 = dccPacket[3];
      decoderType = 0;
    }
    else { //bit7=1 AND bit6=0 -> Accessory Decoder
      decoderAddress = dccPacket[1]&B00111111;
      instrByte1 = dccPacket[2];
      decoderType = 1;
    }
  }
  if (decoderType) { // Accessory Decoder
    if (showAcc) {
      digitalWrite(13, HIGH);
      LED_off_time = millis() + (unsigned long)LED_ON_TIME;
      if (instrByte1&B10000000) { // Basic Accessory Decoder Packet Format
        decoderAddress = (((~instrByte1)&B01110000)<<2) + decoderAddress;
        byte port = (instrByte1&B00000110)>>1;
        Serial.print("Acc ");
        printWithLeadingZeros4((decoderAddress-1)*4 + port + 1);
        Serial.print(" ");
        printWithLeadingZeros4(decoderAddress);
        Serial.print(":");
        Serial.print(port);
        Serial.print(" ");
        if (pktByteCount==3) { // Basic Accessory Decoder Packet Format
          Serial.print(bitRead(instrByte1,0));
          Serial.print(" ");
          if (bitRead(instrByte1,3)) Serial.print(1); // bit 3 is used for solenoid output pulse
          else Serial.print(0);
        } 
        // handle decoder CV access
        else if (pktByteCount==5 && (dccPacket[2]&B10001100)==B00001100) { // Accessory decoder Configuration variable Instruction - backward compatibility
          Serial.print("CV ");
          int cvAddress = 1 + dccPacket[3] + (dccPacket[2]&B00000011)*256;
          byte cvValue = dccPacket[4];
          displayCV(3, cvAddress, cvValue);
        } 
        else if (pktByteCount==6 && (dccPacket[3]&B11110000)==B11100000) { // Accessory decoder Configuration variable Instruction
          Serial.print(" CV ");
          byte cvMode = (dccPacket[3]&B00001100)>>2;
          int cvAddress = 1 + dccPacket[4] + (dccPacket[3]&B00000011)*256;
          byte cvValue = dccPacket[5];
          displayCV(cvMode, cvAddress, cvValue);
        } 
        else {
          Serial.print(" unknown");
        }
      }
      else { // Accessory Extended NMRA spec is not clear about address and instruction format !!!
        Serial.print("Acc Ext ");
        decoderAddress = (decoderAddress<<5) + ((instrByte1&B01110000)>>2) + ((instrByte1&B00000110)>>1);
        Serial.print(decoderAddress);
        Serial.print(" Asp ");
        Serial.print(dccPacket[3],BIN);
      }
    }
  }
  else { // Loc - Multi Function Decoder
    if (showLoc) {  
      Serial.print("Loc ");
      printWithLeadingZeros4(decoderAddress);
      Serial.print(" ");
      byte instructionType = instrByte1>>5;
      switch (instructionType) {

        case 0: // Control
          Serial.print("Control");
        break;

        case 1: // Advanced Operations
          if (instrByte1==B00111111) { // 128 speed steps
            if (bitRead(dccPacket[pktByteCount-1],7)) Serial.print(">> 128");
            else Serial.print("<< 128");
            byte speed = dccPacket[pktByteCount-1]&B01111111;
            if (!speed) Serial.print("Stop");
            else if (speed==1) Serial.print("E-stop");
            else Serial.print(speed-1);
          }
          else if (instrByte1==B00111110) { // Speed Restriction
          if (bitRead(dccPacket[pktByteCount-1],7)) Serial.print("On");
            else Serial.print("Off");
            Serial.print(dccPacket[pktByteCount-1])&B01111111;
          }
        break;

        case 2: // Reverse speed step
          speed = ((instrByte1&B00001111)<<1) - 3 + bitRead(instrByte1,4);
          if (speed==253 || speed==254) Serial.print("Stop");
          else if (speed==255 || speed==0) Serial.print("E-Stop");
          else {
            Serial.print("<< ");
            Serial.print(speed);
          }
        break;

        case 3: // Forward speed step
          speed = ((instrByte1&B00001111)<<1) - 3 + bitRead(instrByte1,4);
          if (speed==253 || speed==254) Serial.print("Stop");
          else if (speed==255 || speed==0) Serial.print("E-Stop");
          else {
            Serial.print(">> ");
            Serial.print(speed);
          }
        break;

        case 4: // Loc Function 1234L
          Serial.print("F1234L ");
//          Serial.print(instrByte1&B00011111,BIN);
          printLocFunctions(instrByte1, 5);
        break;

        case 5: // Loc Function 5-8
          if (bitRead(instrByte1,4)) {
            Serial.print("F5678  ");
//            Serial.print(instrByte1&B00001111,BIN);
          printLocFunctions(instrByte1, 4);
          }
          else { // Loc Function 12-11-10-9
            Serial.print("F9-12  ");
//            Serial.print(instrByte1&B00001111,BIN);
          printLocFunctions(instrByte1, 4);
          }
        break;

        case 6: // Future Expansions
          switch (instrByte1&B00011111) {
            case 0: // Binary State Control Instruction long form
              Serial.print(" BinStateLong ");
              Serial.print(256 * dccPacket[pktByteCount-1] + (dccPacket[pktByteCount-2]&B01111111));
              if bitRead(dccPacket[pktByteCount-2],7) Serial.print(" On ");
              else Serial.print(" Off ");
            break;
            case B00011101: // Binary State Control
              Serial.print("B ");
              Serial.print(dccPacket[pktByteCount-1]&B01111111);
              if bitRead(dccPacket[pktByteCount-1],7) Serial.print(" 1");
              else Serial.print(" 0");
            break;
            case B00011110: // F13-F20 Function Control
              Serial.print("F13-21 ");
//              Serial.print(dccPacket[pktByteCount-1],BIN);
              printLocFunctions(dccPacket[pktByteCount-1], 8);
            break;
            case B00011111: // F21-F28 Function Control
              Serial.print("F21-28 ");
//              Serial.print(dccPacket[pktByteCount-1],BIN);
              printLocFunctions(dccPacket[pktByteCount-1], 8);
            break;
          }
        break;

        case 7:
          Serial.print(" CV ");
          if (instrByte1&B00010000) { // CV Short Form
            byte cvType=instrByte1&B00001111;
            switch (cvType) {
              case B00000010:
                Serial.print("23 ");
                Serial.print(dccPacket[pktByteCount-1]);
              break;
              case B00000011:
                Serial.print("24 ");
                Serial.print(dccPacket[pktByteCount-1]);
              break;
              case B00001001:
                Serial.print("Decoder Lock ");
                Serial.print(dccPacket[pktByteCount-1]);
              break;
            }
          }
          else { // CV Long Form
            int cvAddress = 256 * (instrByte1&B00000011) + dccPacket[pktByteCount-2] + 1;
            byte cvMode = (instrByte1&B00001100)>>2;
            byte cvData = dccPacket[pktByteCount-1];
            switch (cvMode) {
              case 1: // Verify Byte
              case 3: // Write Byte
                displayCV(cvMode, cvAddress, cvData);
              break;
              case 2: // Bit Write
                if (dccPacket[pktByteCount-2]&B00010000) displayCV(11, cvAddress, cvData); // bit verify
                else displayCV(11, cvAddress, cvData); // bit write
              break;
            }
          }
        break;
      }
    }
  }
  Serial.println();
}

void setup() {
// OLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLEDOLED
//  oled.begin(&Adafruit128x32, I2C_ADDRESS);
//  oled.setFont(Adafruit5x7);
//  oled.setCursor(1,0);
//  oled.println("DCC Analyzer");
  Serial.begin(115200);
  Serial.println("---");
  Serial.println("DCC Analyzer");
  Serial.println("---");
  delay(2000);
  pinMode(2, INPUT);
  pinMode(NOT_SHOWACC_PIN, INPUT_PULLUP);
  pinMode(NOT_SHOWLOC_PIN, INPUT_PULLUP);
  pinMode(SHOWVT_PIN,      INPUT_PULLUP);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); //on board LED off
  beginBitDetection(); //Uncomment this line when on DCC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void loop() {

  checkUserInput();

  if(millis() > LED_off_time) digitalWrite(13, LOW); // turn DCC indicator LED off

  /* Dummy packet for test purposes. Comment when on DCC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Loc 1782 CV Write 3 128
  dccPacket[0]=6;
  dccPacket[1]=B11000111;
  dccPacket[2]=B01101110;
  dccPacket[3]=B11101100;
  dccPacket[4]=B00000011;
  dccPacket[5]=B10000000;
  dccPacket[6]=B11111111;
  */
  getPacket(); // Uncomment when on DCC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  pktByteCount = dccPacket[0];
  if (!pktByteCount) return; // No new DCC packet available

// A new DCC packet came in

  for (byte i=1; i<=pktByteCount; i++) checksum ^= dccPacket[i];
  //checksum=0; //Comment this line when on DCC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if (checksum)                return; // Invalid Checksum
  if (dccPacket[1]==B11111111) return; // Idle packet
  
// The new DCC packet is non-idle and has a correct checksum

// Check if the new DCC packet already is in the buffer
  isSameDCCPacket = 0; // check if DCC packet already is in the buffer
  for(byte i=0; i<PACKETBUFFERSIZE; i++) {
    isSameByte = 0;
    for(byte j=1; j<=pktByteCount; j++)
      if(dccPacket[j] == DCCcommand[i][j]) isSameByte++;
    if(isSameByte == pktByteCount) isSameDCCPacket = 1;
  }

  if (!isSameDCCPacket) { // The DCC packet is new, not yet in the buffer
    for (byte i=0; i<=pktByteCount; i++)
      DCCcommand[bufferCounter][i] = dccPacket[i]; // store the packet in the buffer
    evaluateAndPrint();
    bufferCounter = (bufferCounter + 1) % PACKETBUFFERSIZE;
  }

  if(millis() > timeToRefresh) { // clear the buffer
    timeToRefresh = millis() + REFRESHTIME * 1000UL;
    for(byte i=0; i<PACKETBUFFERSIZE; i++)
      for(byte j=0; j<=6; j++)
        DCCcommand[i][j] = 0;
  Serial.println("---");
  }
} // END loop
