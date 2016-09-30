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
PTT out      B5   | [ ]D13/SCK        MISO/D12[ ] |   B4 PA Bias   (Output)
                  | [ ]3.3V           MOSI/D11[ ]~|   B3 Lclock    (Output) to Pin 11 of 74HC595
                  | [ ]V.ref     ___    SS/D10[ ]~|   B2 Llatch    (Output) to Pin 12 of 74HC595
USER0        C0   | [ ]A0       / N \       D9[ ]~|   B1 Ldata     (Output) to Pin 14 of 74HC595
USER1        C1   | [ ]A1      /  A  \      D8[ ] |   B0 Filter 6  (Output)
USER2        C2   | [ ]A2      \  N  /      D7[ ] |   D7 Filter 5  (Output)
USER3        C3   | [ ]A3       \_0_/       D6[ ]~|   D6 Filter 4  (Output)
I2C Bus      C4   | [ ]A4/SDA               D5[ ]~|   D5 Filter 3  (Output)
I2C Bus      C5   | [ ]A5/SCL               D4[ ] |   D4 Filter 2  (Output)
Latch OP enable   | [ ]A6              INT1/D3[ ]~|   D3 Filter 1  (Output)
N/C               | [ ]A7              INT0/D2[ ] |   D2 PTT in    (Input)
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

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

uint8_t * heapptr, * stackptr;  // I declared these globally for memory checks


// Latch stuff

// Shift Register for Rx Filter Pin assignments
#define outputEnable A6     // Pin 11 of 74HC595 U5 to pin 7 of Arduino Nano
#define Lclock       11     // Pin 11 of 74HC595 U3 to pin 14 of Arduino Nano
#define Llatch       10     // Pin 12 of 74HC595 U4 to pin 13 of Arduino Nano
#define Ldata         9     // Pin 14 of 74HC595 U3 to pin 12 of Arduino Nano
/*
  // Set the C Relays from _status.C_relays;
  digitalWrite(Llatch, LOW);
  shiftOut(Ldata, Lclock, MSBFIRST, Cmap); // send this binary value to the Capacitor shift register

  // Set the L Relays from _status.L_relays;
  temp = bitRead(Lmap, 7);
  Lmap = Lmap << 1;
  bitWrite(Lmap, 0, temp);
  shiftOut(Ldata, Lclock, MSBFIRST, Lmap); // send this binary value to the Inductor shift register
  digitalWrite(Llatch, HIGH);
*/


// Debug Defines follow here


// Option defines follow here
#define OPTION_Invert_Inputs    // Allows inverting or non-inverting buffers from HL to Arduino

// PIN ASSIGNMENT defines follow here

// Input Pins - Analog used as digital
#define user0          A0 // Connected to CN2 pin 3, USER0  
#define user1          A1 // Connected to CN2 pin 2, USER1
#define user2          A2 // Connected to CN2 pin 1, USER2
#define user3          A3 // Connected to CN2 pin 7, USER3
//#define pttIn          2  // Connected to CN2 pin 4, PTT#
// Input Pins - Analog used as analog
#define fwdPwr         A6 // Connected to users power output metering
#define swr            A7 // Connected to users power output metering
#define pttIn          2  // Connected to CN2 pin 4, PTT#

/* Not enough available input pins to connect these at present
#define user4          d3 // Connected to CN2 pin 8, USER4
#define user5          d3 // Connected to CN2 pin 9, USER5
#define user6          d3 // Connected to CN2 pin 10, USER6
#define swr            A7 // Connected to users swr metering
#define swr            A2 // Connected to users rotator position potentiometer
*/

// Output Pins
#define ThroughFilter  3
#define Filter1        4
#define Filter2        5
#define Filter3        6
#define Filter4        7
#define Filter5        8
#define Filter6        9
#define Filter7        10
#define paBias         11
#define pttOut         13
#define LEDpin         13   // A LED is connected to this pin, use for heartbeat

// I/O Pins
#define I2C_sda        A4
#define I2C_scl        A5

// System constants defined here (TODO convert to statics)
#define PA_biasDelay 20    //PA bias delay in mSec

// Setup LCD stuff for 16 col x 2 row display
#define lcdNumCols 16 // -- number of columns in the LCD
#define lcdNumRows  2 // -- number of rows in the LCD
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
// Bar part block characters definitions. Use lcd.write(6) for an empty bar
byte p1[8] = {
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

// Global variables always start with an underscore


/**********************************************************************************************************/

void setup() {
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
//  digitalWrite(pttIn, HIGH);

// Setup outputs. Any defined outputs default to LOW (false or 0)
  pinMode(pttOut, OUTPUT);
  pinMode(ThroughFilter, OUTPUT);
  pinMode(Filter1, OUTPUT);
  pinMode(Filter2, OUTPUT);
  pinMode(Filter3, OUTPUT);
  pinMode(Filter4, OUTPUT);
  pinMode(Filter5, OUTPUT);
  pinMode(Filter6, OUTPUT);
  pinMode(Filter7, OUTPUT);
  pinMode(paBias, OUTPUT);

  
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
  digitalWrite(LEDpin, HIGH);  //Turn LED on

  //Initialize serial and wait for port to open:
  Serial.begin(115200); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(F("Arduino Hermes-Lite control line expander ver 0.1.0"));
  Serial.println(F("Copyright (C) 2015, Graeme Jury ZL2APV"));
  Serial.print(F("available RAM = "));
  Serial.println(freeRam());
  Serial.println();
}
/**********************************************************************************************************/
/* The main loop spends most of its time polling the control data lines from Hermes-Lite. Subroutines
   should be written to be non blocking. PTT is on an interrupt for instant response.
*/   
void loop() {
  // bit values for fpgaState ...
  // bit 0 holds USER0 value (0 or 1)
  // similar until
  // bit 6 holds USER6 value (0 or 1)
  // bit 7 holds PTT# value (0 or 1)
  static byte fpgaState = 0; // On Rx, with through filter selected
  
  byte fpgaStateTmp;
  byte cnt;
  boolean pttChanged;
  boolean filtersChanged;

  // Read the inputs from fpga and temporary store
  // DEBUG I have inverted inputs as they are held Hi by pullups
  cnt = 0;
#ifdef OPTION_Invert_Inputs
  bitWrite(fpgaStateTmp, cnt++, !digitalRead(user0));
  bitWrite(fpgaStateTmp, cnt++, !digitalRead(user1));
  bitWrite(fpgaStateTmp, cnt++, !digitalRead(user2));
  bitWrite(fpgaStateTmp, cnt++, !digitalRead(user3));
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt, !digitalRead(pttIn));
#else
  bitWrite(fpgaStateTmp, cnt++, digitalRead(user0));
  bitWrite(fpgaStateTmp, cnt++, digitalRead(user1));
  bitWrite(fpgaStateTmp, cnt++, digitalRead(user2));
  bitWrite(fpgaStateTmp, cnt++, digitalRead(user3));
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt++, 0);
  bitWrite(fpgaStateTmp, cnt, digitalRead(pttIn));
#endif

/* We could be working crossband so if going from Rx to Tx it is important to switch the filters
   and then key the transmitter. Otherwise from Tx to Rx we unkey transmitter then switch filters.
*/
  // See if the ptt has changed and flag if so
  pttChanged = false;
  if(bitRead(fpgaState, 7) != bitRead(fpgaStateTmp, 7)) pttChanged = true;
  
  // See if the filter selection has changed and flag if so
  filtersChanged = false;
  if((fpgaState & B01111111) != (fpgaStateTmp & B01111111)) {
    filtersChanged = true;
//    Serial.print("the value of user2 = "); Serial.println(digitalRead(user2));
    Serial.print("the value of fpgaState = "); Serial.print(fpgaState);
    Serial.print(" and fpgaStateTmp = "); Serial.println(fpgaStateTmp);
  }  
  // Here is where we process any changed condition from Hermes-Lite
  if(pttChanged) {
    if(bitRead(fpgaStateTmp, 7)) { //We are going from Rx to Tx
      if(filtersChanged) { // Switch them first
        switchFilters(fpgaStateTmp & B01111111);
      }
      digitalWrite(pttOut, HIGH);
      delay(PA_biasDelay);
      digitalWrite(paBias, HIGH);
    }
    else { // We are going from Tx to Rx
      digitalWrite(paBias, LOW);
      digitalWrite(pttOut, LOW);
      if(filtersChanged) { // Set correct Rx filter
        switchFilters(fpgaStateTmp & B01111111);
      }
    }
    fpgaState = fpgaStateTmp;
    filtersChanged = false;
    pttChanged = false;
  }
  if(filtersChanged) {
    if(bitRead(fpgaStateTmp, 7)) { //Go back to Rx as switching filters while Tx is an error condition
      digitalWrite(paBias, LOW);
      digitalWrite(pttOut, LOW);
    }
    switchFilters(fpgaStateTmp & B01111111);
    fpgaState = fpgaStateTmp;
  }
  fpgaState = fpgaStateTmp;
  filtersChanged = false;
  pttChanged = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subroutines start here
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void switchFilters(byte filterNum) {
  // In final version translation tables would be used here but I am simply
  // using filters 1 to 6 as a straight input to output bcd decode
  digitalWrite(ThroughFilter, LOW); // Clear the previous filters
  digitalWrite(Filter1, LOW);
  digitalWrite(Filter2, LOW);
  digitalWrite(Filter3, LOW);
  digitalWrite(Filter4, LOW);
  digitalWrite(Filter5, LOW);
  digitalWrite(Filter6, LOW);
  digitalWrite(Filter7, LOW);
//  digitalWrite(Filter8, LOW);
  digitalWrite(LEDpin, LOW);
  
  Serial.print("filterNum = "); Serial.println(filterNum);
  switch (filterNum) {
    case 0:
      digitalWrite(ThroughFilter, HIGH);
      break;
    case 1:
      digitalWrite(Filter1, HIGH);
      break;
    case 2:
      digitalWrite(LEDpin, HIGH);
      break;
    case 3:
      digitalWrite(Filter3, HIGH);
      break;
    case 4:
      digitalWrite(Filter4, HIGH);
      break;
    case 5:
      digitalWrite(Filter5, HIGH);
      break;
    case 6:
      digitalWrite(Filter6, HIGH);
      break;
    case 7:
      digitalWrite(Filter7, HIGH);
      break;
    case 11:
      digitalWrite(LEDpin, HIGH);
      break;
    default:
      digitalWrite(ThroughFilter, HIGH);  
  }      
}

/**********************************************************************************************************/

void switchRxFilters(byte filterNum) {
  
/*
  switch (filterNum) {
    case 0:
      digitalWrite(ThroughFilter, HIGH);
      break;
    case 1:
      digitalWrite(Filter1, HIGH);
      break;
    case 2:
      digitalWrite(LEDpin, HIGH);
      break;
    case 3:
      digitalWrite(Filter3, HIGH);
      break;
    case 4:
      digitalWrite(Filter4, HIGH);
      break;
    case 5:
      digitalWrite(Filter5, HIGH);
      break;
    case 6:
      digitalWrite(Filter6, HIGH);
      break;
    case 7:
      digitalWrite(Filter7, HIGH);
      break;
    case 11:
      digitalWrite(LEDpin, HIGH);
      break;
    default:
      digitalWrite(ThroughFilter, HIGH);  
  } 
*/       
  digitalWrite(Llatch, LOW);
  shiftOut(Ldata, Lclock, MSBFIRST, filterNum); // send this binary value to the Capacitor shift register
  digitalWrite(Llatch, HIGH);
}

  /**********************************************************************************************************/

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

/**********************************************************************************************************/

void lcdPrintSplash()
{
  lcd.home();                   // go home
  lcd.print(F("CMD EXPANDER by "));  
  lcd.setCursor (0, 1);        // go to the next line
  lcd.print(F("ZL2APV (c) 2015 "));
}

/**********************************************************************************************************/

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

/**********************************************************************************************************/



