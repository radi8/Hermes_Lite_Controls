//////////////////////////////////////////////////////////////////
// Copyright ©2015 Graeme Jury ZL2APV
// Released under the GPL v2 License - Please alter and share.
// Control lines expansion for Hermes-Lite
//////////////////////////////////////////////////////////////////

/*
The Hermes-Lite project was first pioneered by Steve Haynal KF7O and the first set of boards and practical
build was pioneered by Rob Frohne KL7NA. From there numerous others have joined in to create this game
changing project.

RESOURCES
Hermes-Lite wiki            ... https://github.com/softerhardware/Hermes-Lite/wiki
Hermes-Lite circuit diagram ... https://github.com/softerhardware/Hermes-Lite/blob/master/pcb/hermeslite.pdf
Front end circuit diagram   ... https://github.com/softerhardware/Hermes-Lite/raw/master/frontend/basic/frontend.pdf
HL Protocol matching Hermes ... https://github.com/softerhardware/Hermes-Lite/wiki/Protocol-Coverage
John Williams J16 proposal  ... https://docs.google.com/viewer?a=v&pid=forums&srcid=MTY4NjY5MjUzNTgyMDIxNjk5MDkBMDY0MDk1MDI1ODgzNzI5ODYwNDMBd0VrOTVFUVQ5Q1VKATAuMQEBdjI
Arduino Nano Manual         ... https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf
*/
/*
                               +-----+
                  +------------| USB |------------+
                  |            +-----+            |
PTT out      B5   | [ ]D13/SCK        MISO/D12[ ] |   B4 PA Bias      (Output)
                  | [ ]3.3V           MOSI/D11[ ]~|   B3 Lclock       (Output) to Pin 11 of 74HC595
                  | [ ]V.ref     ___    SS/D10[ ]~|   B2 Llatch       (Output) to Pin 12 of 74HC595
USER0        C0   | [ ]A0       / N \       D9[ ]~|   B1 LatchEnable  (Output) to Pin 13 of 74HC595
USER1        C1   | [ ]A1      /  A  \      D8[ ] |   B0 Ldata        (Output) to Pin 14 of 74HC595
USER2        C2   | [ ]A2      \  N  /      D7[ ] |   D7 Unused       (Output)
USER3        C3   | [ ]A3       \_0_/       D6[ ]~|   D6 Filter 4     (Output)
I2C Bus      C4   | [ ]A4/SDA               D5[ ]~|   D5 Filter 3     (Output)
I2C Bus      C5   | [ ]A5/SCL               D4[ ] |   D4 Filter 2     (Output)
N/C               | [ ]A6              INT1/D3[ ]~|   D3 Filter 1     (Output)
N/C               | [ ]A7              INT0/D2[ ] |   D2 PTT in       (Input)
                  | [ ]5V                  GND[ ] |     
             C6   | [ ]RST                 RST[ ] |   C6
                  | [ ]GND   5V MOSI GND   TX1[ ] |   D0
+12v Pwr          | [ ]Vin   [ ] [ ] [ ]   RX1[ ] |   D1
                  |          [ ] [ ] [ ]          |
                  |          MISO SCK RST         |
                  | NANO-V3                       |
                  +-------------------------------+
         
                  http://busyducks.com/ascii-art-arduinos
*/

#include "Features_and_Settings.h"
#if defined(FEATURE_I2C_LCD)
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
#endif

uint8_t * heapptr, * stackptr;  // I declared these globally for memory checks

#define CODE_VERSION "0.5.20161122"


// Latch stuff

// Shift Register for Rx Filter Pin assignments
#define Lclock       11     // Pin 11 of 74HC595 to pin 14 of Arduino Nano
#define Llatch       10     // Pin 12 of 74HC595 to pin 13 of Arduino Nano
#define LEnable       9     // Pin 13 of 74HC595 to pin 11 of Arduino Nano
#define Ldata         8     // Pin 14 of 74HC595 to pin 12 of Arduino Nano



// PIN ASSIGNMENT defines follow here

// Input Pins - Analog used as digital
#define user0          A0 // Connected to CN2 pin 3, USER0  
#define user1          A1 // Connected to CN2 pin 2, USER1
#define user2          A2 // Connected to CN2 pin 1, USER2
#define user3          A3 // Connected to CN2 pin 7, USER3
// Input Pins - Analog used as analog
#define fwdPwr         A6 // Connected to users power output metering
#define swr            A7 // Connected to users power output metering
#define pttIn          2  // Connected to CN2 pin 4, PTT#

// Output Pins
#define Filter1        3 // Data 3 = Bit 0 of Tx Filter output, connects to Filter 1 line
#define Filter2        4
#define Filter3        5
#define Filter4        6 // Data 6 = Bit 3 of Tx Filter output, connects to Filter 4 line

#define paBias         12
#define pttOut         13
#define LEDpin         13   // A LED is connected to this pin, use for heartbeat

// I/O Pins
#define I2C_sda        A4
#define I2C_scl        A5

// System constants defined here
const uint8_t PA_biasDelay = 20;    //PA bias delay in mSec
const uint8_t relaySettleTime = 10;

#if defined(FEATURE_I2C_LCD)
  // Setup LCD stuff for 16 col x 2 row display
  #define lcdNumCols 16 // -- number of columns in the LCD
  #define lcdNumRows  2 // -- number of rows in the LCD
  // set the LCD address to 0x27 for a 20 chars 4 line display
  // Set the pins on the I2C chip used for LCD connections:
  //                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
  // Bar part block characters definitions. Use lcd.write(6) for an empty bar
  byte p1[8] = {found
    0x00,0x00,0x10,0x10,0x10,0x10,0x00,0x00}; // 1 part of bar block
  byte p2[8] = {
    0x00,0x00,0x18,0x18,0x18,0x18,0x00,0x00}; // 2
  byte p3[8] = {
    0x00,0x00,0x1C,0x1C,0x1C,0x1C,0x00,0x00}; // 3
  byte p4[8] = {
    0x00,0x00,0x1E,0x1E,0x1E,0x1E,0x00,0x00}; // 4 parts of bar block
  byte p5[8] = {
    0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x00,0x00}; // full bar block
  byte p6[8] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // blank bar block
#endif

// Global variables always start with a G underscore e.g. G_myGlobal


/**********************************************************************************************************/

void setup() {
// Setup Latch pins 
  pinMode(LEnable, OUTPUT); // Switch off HiZ mode for shift register
  digitalWrite(LEnable, LOW);
  pinMode(Lclock, OUTPUT);
  pinMode(Llatch, OUTPUT);
  pinMode(Ldata , OUTPUT);
   
// Setup inputs. Any defined inputs default to no pullups enabled
  pinMode(user0, INPUT);
  pinMode(user1, INPUT);
  pinMode(user2, INPUT);
  pinMode(user3, INPUT);
  pinMode(pttIn, INPUT);
/* Not enough available input pins to connect these at present
  pinMode(user4, INPUT);
  pinMode(user5, INPUT);
  pinMode(user6, INPUT);
*/
  digitalWrite(user0, HIGH); // Enable pullups for testing
  digitalWrite(user1, HIGH);
  digitalWrite(user2, HIGH);
  digitalWrite(user3, HIGH);
  digitalWrite(pttIn, HIGH);

// Setup outputs. Any defined outputs default to LOW (false or 0)
  pinMode(pttOut, OUTPUT);
  pinMode(Filter1, OUTPUT);
  pinMode(Filter2, OUTPUT);
  pinMode(Filter3, OUTPUT);
  pinMode(Filter4, OUTPUT);
  pinMode(paBias, OUTPUT);

  digitalWrite(pttOut, LOW); // Initially in Rx mode
  digitalWrite(paBias, LOW); // Initially Finals biased off

#ifdef FEATURE_I2C_LCD 
  lcd.begin(lcdNumRows, lcdNumCols);
  //  lcd.clear(); //TODO check if this can be removed as splash will write whole screen
  // -- do some delay: some times I've got broken visualization
  delay(100);
  lcd.createChar(1, p1);
  lcd.createChar(2, p2);
  lcd.createChar(3, p3);
  lcd.createChar(4, p4);
  lcd.createChar(5, p5);
  lcd.createChar(6, p6);
  lcdPrintSplash();
#endif

#ifdef DEBUG_ENABLED
//Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(F("Arduino Hermes-Lite control line expander"));
  Serial.println(F("Copyright (C) 2015, Graeme Jury ZL2APV"));
  Serial.print(F("Code Version = "));
  Serial.println(CODE_VERSION);
  Serial.print(F("available RAM = "));
  Serial.println(freeRam());
  Serial.println();
#endif  
}
/**********************************************************************************************************/
/* The main loop spends most of its time polling the control data lines from Hermes-Lite. Subroutines
   should be written to be non blocking. PTT is on an interrupt for instant response.
*/   
void loop() {
  // bit values for controlData ...
  // bit 0 holds USER0 value (0 or 1)
  // similar until
  // bit 6 holds USER6 value (0 or 1)
  // bit 7 holds PTT# value (0 or 1)
  static byte controlData = 0; // Initially on Rx, with 160M HP & 12_10 LP selected
  static boolean pttChanged = true;  // Force a state write to the ptt line on switch on
  static boolean filtersChanged = true; // and same for the Tx and Rx filters
    
  uint8_t controlDataTmp = 0;
  uint8_t cnt = 0;

// Read the inputs from radio hardware and temporary store in "controlDataTmp"
  cnt = 0;
  
#ifdef FEATURE_Invert_Inputs
// Note: I have inverted inputs as they are held Hi by pullups
  bitWrite(controlDataTmp, cnt++, !digitalRead(user0));
  bitWrite(controlDataTmp, cnt++, !digitalRead(user1));
  bitWrite(controlDataTmp, cnt++, !digitalRead(user2));
  bitWrite(controlDataTmp, cnt++, !digitalRead(user3));
//  bitWrite(controlDataTmp, cnt++, !digitalRead(user4));
//  bitWrite(controlDataTmp, cnt++, !digitalRead(user5));
//  bitWrite(controlDataTmp, cnt++, !digitalRead(user6));
#else
  bitWrite(controlDataTmp, cnt++, digitalRead(user0));
  bitWrite(controlDataTmp, cnt++, digitalRead(user1));
  bitWrite(controlDataTmp, cnt++, digitalRead(user2));
  bitWrite(controlDataTmp, cnt++, digitalRead(user3));
//  bitWrite(controlDataTmp, cnt++, digitalRead(user4));
//  bitWrite(controlDataTmp, cnt++, digitalRead(user5));
//  bitWrite(controlDataTmp, cnt++, digitalRead(user6));
#endif

#ifdef FEATURE_Invert_ptt_line
  bitWrite(controlDataTmp, 7, !digitalRead(pttIn));
#else
  bitWrite(controlDataTmp, 7, digitalRead(pttIn));
#endif


/* We could be working crossband so if going from Rx to Tx it is important to switch the filters
   and then key the transmitter. Otherwise from Tx to Rx we unkey transmitter then switch filters.
*/
// See if the ptt has changed and flag if so

  if(bitRead(controlData, 7) != bitRead(controlDataTmp, 7)) pttChanged = true;
  
// See if the filter selection has changed and flag if so

  if((controlData & B00111111) != (controlDataTmp & B00111111)) {
    filtersChanged = true;
  #ifdef DEBUG_BAND_CHANGE
    Serial.print(F("The band has changed, the button pressed has a value of "));
    Serial.println(controlDataTmp & B00111111);
    Serial.print(F("Input word (controlDataTmp) = ")); // Bit 7 is ptt bit 0-5 = User0 to User5
    print_binary(controlDataTmp, 8); Serial.println();
    Serial.print(F("Last state  ( controlData ) = "));
    print_binary(controlData, 8); Serial.println(); 
  #endif    
  }

// Here is where we process any changed condition from Hermes-Lite
  if(pttChanged) {
    if(bitRead(controlDataTmp, 7)) { //We are going from Rx to Tx
      if(filtersChanged) { // Switch them first
        switchFilters(controlDataTmp & B01111111);
      }
      digitalWrite(pttOut, HIGH);
      delay(PA_biasDelay);
      digitalWrite(paBias, HIGH);
#ifdef DEBUG_PTT_CHANGE
  Serial.println(F("Going to TRANSMIT"));
#endif  
    }
    else { // We are going from Tx to Rx
      digitalWrite(paBias, LOW);
      digitalWrite(pttOut, LOW);
      if(filtersChanged) { // Set correct Rx filter
        switchFilters(controlDataTmp & B00111111);
      }
#ifdef DEBUG_PTT_CHANGE
  Serial.println(F("Going to RECEIVE"));
#endif      
    }
    controlData = controlDataTmp;
    filtersChanged = false;
    pttChanged = false;
  }
  if(filtersChanged) {
    if(bitRead(controlDataTmp, 7)) { //Go back to Rx as switching filters while Tx is an error condition
      digitalWrite(paBias, LOW);
      digitalWrite(pttOut, LOW);
    }
    switchFilters(controlDataTmp & B00111111);
    controlData = controlDataTmp;
  }
  controlData = controlDataTmp;
  filtersChanged = false;
  pttChanged = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subroutines start here
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void switchFilters(byte bandNum) {

  uint8_t txFilter;
  uint8_t rxFilter;
  
  txFilter = txFilterMap[bandNum],
  rxFilter = rxFilterMap[bandNum],

#ifdef DEBUG_SHOW_FILTER_SWITCH_SIGNALS
  Serial.println();
  Serial.println(F("void switchFilters(byte bandNum)"));
  Serial.print(F("txFilter = ")); print_binary(txFilter, 6); Serial.print(F("; and rxFilter = ")); print_binary(rxFilter, 6);
  Serial.println();
#endif  

  // Shift the rxFilter data to the highest 6 bits as we are going to put bit 6 and 5 of the TX Filters
  // into the 1st 2 locations

  rxFilter = rxFilter << 2;
  bitWrite(rxFilter, 1, bitRead(txFilter, 5));
  bitWrite(rxFilter, 0, bitRead(txFilter, 4));
  
  txFilter = txFilter & B00001111; // Leave only the first 4 bits

  digitalWrite(Llatch, LOW);
  shiftOut(Ldata, Lclock, MSBFIRST, rxFilter); // send rxFilter signals and top 2 bits of txFilter signals.
  digitalWrite(Llatch, HIGH);
  if(bitRead(txFilter, 0)) digitalWrite(Filter1, HIGH); // copy bit 0 to Digital output
  else digitalWrite(Filter1, LOW);
  if(bitRead(txFilter, 1)) digitalWrite(Filter2, HIGH); // and next 3 bits too.
  else digitalWrite(Filter2, LOW);
  if(bitRead(txFilter, 2)) digitalWrite(Filter3, HIGH);
  else digitalWrite(Filter3, LOW);
  if(bitRead(txFilter, 3)) digitalWrite(Filter4, HIGH);
  else digitalWrite(Filter4, LOW);
  
#ifdef DEBUG_SHOW_FILTER_SWITCH_SIGNALS
    Serial.print(F("Parameter 'bandNum' = "));  Serial.println(bandNum);
    Serial.print(F("Tx Filter output = "));
    print_binary(txFilter, 4);
    Serial.print(F(", Rx Filter output = "));
    print_binary(rxFilter, 8); Serial.println();
    Serial.println(F("----------------------------"));
#endif  
}

/**********************************************************************************************************/

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

/**********************************************************************************************************/
#if defined(FEATURE_I2C_LCD)
void lcdPrintSplash()
{
  lcd.home();                   // go home
  lcd.print(F("CMD EXPANDER by "));  
  lcd.setCursor (0, 1);        // go to the next line
  lcd.print(F("ZL2APV (c) 2015 "));
}
#endif
/**********************************************************************************************************/
#if defined(FEATURE_I2C_LCD)
/*   Display an analog progress bar/s
 The analog input ranges from 0 to 1023 (1024 values) and on a 16 column display will have a
 value 0f 1024/16 = 64 per column. A number like 400 would be represented by 6 full columns i.e.
 (400/64 = 6.25) which is 6 full columns of dots with a remainder of .25 or 64 * .25 = 16. As 
 each column is broken into 5 dots per row so we can represent the partial block value as int(.25*5) = 1.
 Calls to this would be similar to ... displayAnalog(0, 0, value1); displayAnalog(0, 1, value2);

 */
void displayAnalog(byte col, byte row, int value)
{
  int cnt;
  byte blocks = value / 64;
  byte partBlock = value % 64;
  byte remSpaces = 0; // Number of spaces to write to overwrite old data in the line

  lcd.setCursor(col,row);
  // Calculate how many blank blocks to write to overwrite old data when bargraph is less than full row.
  if(blocks < 15) remSpaces = 16 - (blocks + 1); // We will tack a part block on end of 15 or less

  // Print out the full blocks in bargraph
  for(cnt = 1; cnt < (blocks + 1); cnt++) {
    lcd.write(5);
  }

  // If < 16 full blocks print out the part block of barcode
  if(blocks < 16) {
    if(partBlock < 7)     lcd.write(6); // value too small to show as part block so print blank.
    else if(partBlock < 20) lcd.write(1);
    else if(partBlock < 33) lcd.write(2);
    else if(partBlock < 45) lcd.write(3);
    else if(partBlock < 58) lcd.write(4);
    else lcd.write(5); // Value so close to full block we will show it as so.
  }

  // Now blank rest of blocks in row so barcode not corrupted by old data.
  for(cnt = 0; cnt < remSpaces; cnt++) {
    lcd.write(6);
  }
}
#endif
/**********************************************************************************************************/

// PRINT_BINARY - Arduino
//
// Prints a positive integer in binary format with a fixed withdth
//
// copyright, Peter H Anderson, Baltimore, MD, Nov, '07

void print_binary(int v, int num_places)
{
  int mask = 0, n;

  for (n = 1; n <= num_places; n++) {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask;  // truncate v to specified number of places
  while (num_places) {
    if (v & (0x0001 << num_places - 1)) {
      Serial.print(F("1"));
    }
    else {
      Serial.print(F("0"));
    }
    --num_places;
    if (((num_places % 4) == 0) && (num_places != 0)) {
      Serial.print(F("_"));
    }
  }
}

/**********************************************************************************************************/



