#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN A0





int laneCounts[4];
boolean gateState;

 

byte readCard[4];
String tag_UID = "39C3BB99";
String tagID = "";
MFRC522 mfrc522(SS_PIN, RST_PIN);



bool executed = false;

int latchPin = 5;
int clockPin = 6;
int dataPin = 4;

int numOfRegisters = 2;
byte* registerState;

int northRed = 14;
int northYellow = 15;
int northGreen = 0;

int eastRed = 11;
int eastYellow = 12;
int eastGreen = 13;

int southRed = 8;
int southYellow = 9;
int southGreen = 10;

int westRed = 1;
int westYellow = 2;
int westGreen = 3;

boolean emergency = 0;
boolean gate = 0;

unsigned long previousMillis = 0;
int currentState = 0;

int  sensorPin[9] = { A3, A1 , A2, A7, A4, A5, 7 , 8 , 9};
boolean sensorState[9] = {};

unsigned long lastActionTime = 0;
unsigned long actionInterval = 1500;

int cardStatus = 0;
struct lane {

  int count;
} lane[4];


long delayHigh;
long delayLow;


boolean readID();
boolean val = 0;
unsigned long valStartTime = 0;
const unsigned long valDuration = 30000;

void setup() {
  // Initialize array
  registerState = new byte[numOfRegisters];
  for (size_t i = 0; i < numOfRegisters; i++) {
    registerState[i] = 0;
  }

 
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  
  nextState();

  Serial.begin(9600);
  
  SPI.begin();      
  mfrc522.PCD_Init(); 

  for ( int i = 0; i <= 8; i++) {
    pinMode(sensorPin[i], INPUT);
  }

   
}

void loop() {

 if (readID()){
  
    emergency = 1;
    valStartTime = millis(); 
  }

  // Check if it's time to reset val to 0
  if (emergency && millis() - valStartTime >= valDuration)
  {
    emergency = 0;
  
  }

  
  


  unsigned long currentMillis = millis();
  if(lane[0].count == 0 && lane[1].count == 0 && lane[2].count == 0 && lane[3].count == 0){

      regWrite(northRed, LOW);
      regWrite(northYellow, HIGH);
      regWrite(northGreen, LOW);
      regWrite(southRed, LOW);
      regWrite(southYellow, HIGH);
      regWrite(southGreen, LOW);
      regWrite(westRed, LOW);
      regWrite(westYellow, HIGH);
      regWrite(westGreen, LOW);
      regWrite(eastRed, LOW);
      regWrite(eastYellow, HIGH);
      regWrite(eastGreen, LOW);

    
  }
  else{

      if ((lane[1].count <= 5) && (emergency == 1) && !executed) {
    executed = true;
    
    gate = 0;
    
    regWrite(northRed, HIGH);
    regWrite(northYellow, LOW);
    regWrite(northGreen, LOW);
    regWrite(southRed, LOW);
    regWrite(southYellow, LOW);
    regWrite(southGreen, HIGH);
    regWrite(westRed, HIGH);
    regWrite(westYellow, LOW);
    regWrite(westGreen, LOW);
    regWrite(eastRed, HIGH);
    regWrite(eastYellow, LOW);
    regWrite(eastGreen, LOW);
    
    delay(5000);
    emergency = 0;
  }

  else  if ((lane[1].count > 5)&&(emergency == 1)) {

     delayHigh = 10000;
     delayLow = 2000;
    
      
     gate = 1;
  }

   else  if ((lane[1].count > 5)&&(emergency == 0)) {

     delayHigh = 10000;
     delayLow = 2000;
    
     gate = 0;
  }

   else  if ((lane[1].count < 5)&&(emergency == 0)) {

     delayHigh = 4000;
     delayLow = 2000;
    
    
     gate = 0;
  }

 

  switch (currentState) {
    case 0:
      // North-South Green, East-West Red
      if (currentMillis - previousMillis >= delayHigh) {
        nextState();
      }
      break;
    case 1:
      // North-South Yellow, East-West Red
      if (currentMillis - previousMillis >= delayLow) {
        nextState();
      }
      break;
    case 2:
      // North-South Red, East-West Green
      if (currentMillis - previousMillis >= delayLow) {
        nextState();
      }
      break;
    case 3:
      // North-South Red, East-West Yellow
      if (currentMillis - previousMillis >= delayHigh) {
        nextState();
      }
      break;
    case 4:
      // Transition to case 4
      if (currentMillis - previousMillis >= delayLow) {
        nextState();
      }
      break;
    case 5:
      // Transition to case 5
      if (currentMillis - previousMillis >= delayLow) {
        nextState();
      }
      break;
  }


  Serial.print("Lane0:");
  Serial.print(lane[0].count);
  Serial.print(",Lane1:");
  Serial.print(lane[1].count);
  Serial.print(",Lane2:");
  Serial.print(lane[2].count);
  Serial.print(",Lane3:");
  Serial.print(lane[3].count);
  Serial.print(",Emergency:");
  Serial.print(emergency);
  Serial.print(",Gate:");
  Serial.println(gate);
  
 
  }
  //lane1
  if (debounceSensor1(sensorState[0]) == 0 && sensorState[0] == 1)
  {
    lane[0].count++;
    sensorState[0] = 0;
  }
  else if (debounceSensor1(sensorState[0]) == 1 && sensorState[0] == 0)
  {
    sensorState[0] = 1;
  }


  if (debounceSensor2(sensorState[1]) == 0 && sensorState[1] == 1)
  {
    lane[0].count--;
    sensorState[1] = 0;
  }
  else if (debounceSensor2(sensorState[1]) == 1 && sensorState[1] == 0)
  {
    sensorState[1] = 1;
  }


  //lane2
  if (debounceSensor3(sensorState[2]) == 0 && sensorState[2] == 1)
  {
    lane[1].count--;
    sensorState[2] = 0;
  }
  else if (debounceSensor3(sensorState[2]) == 1 && sensorState[2] == 0)
  {
    sensorState[2] = 1;
  }


  if (debounceSensor5(sensorState[4]) == 0 && sensorState[4] == 1)
  {
    lane[1].count++;
    sensorState[4] = 0;
  }
  else if (debounceSensor5(sensorState[4]) == 1 && sensorState[4] == 0)
  {
    sensorState[4] = 1;
  }


  //lane3

  if (debounceSensor6(sensorState[5]) == 0 && sensorState[5] == 1)
  {
    lane[2].count++;
    sensorState[5] = 0;
  }
  else if (debounceSensor6(sensorState[5]) == 1 && sensorState[5] == 0)
  {
    sensorState[5] = 1;
  }


  if (debounceSensor7(sensorState[6]) == 0 && sensorState[6] == 1)
  {
    lane[2].count--;
    sensorState[6] = 0;
  }
  else if (debounceSensor7(sensorState[6]) == 1 && sensorState[6] == 0)
  {
    sensorState[6] = 1;
  }

  //lane4

  if (debounceSensor8(sensorState[7]) == 0 && sensorState[7] == 1)
  {
    lane[3].count++;
    sensorState[7] = 0;
  }
  else if (debounceSensor8(sensorState[7]) == 1 && sensorState[7] == 0)
  {
    sensorState[7] = 1;
  }


  if (debounceSensor9(sensorState[8]) == 0 && sensorState[8] == 1)
  {
    lane[3].count--;
    sensorState[8] = 0;
  }
  else if (debounceSensor9(sensorState[8]) == 1 && sensorState[8] == 0)
  {
    sensorState[8] = 1;
  }

  for (int i = 0; i <= 3; i++) {

    if (lane[i].count < 0) {

      lane[i].count = 0;
    }

  }

 

  
 

}

boolean readID()
{
  
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return false;
  }
  
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return false;
  }
  tagID = "";
  // Read the 4 byte UID
  for (uint8_t i = 0; i < 4; i++)
  {
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); 
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); 
  return true;
}


void nextState() {
  currentState = (currentState + 1) % 6;
  previousMillis = millis();

  switch (currentState) {
    case 0:
      
      regWrite(northRed, LOW);
      regWrite(northYellow, LOW);
      regWrite(northGreen, HIGH);
      regWrite(southRed, LOW);
      regWrite(southYellow, LOW);
      regWrite(southGreen, HIGH);
      regWrite(westRed, HIGH);
      regWrite(westYellow, LOW);
      regWrite(westGreen, LOW);
      regWrite(eastRed, HIGH);
      regWrite(eastYellow, LOW);
      regWrite(eastGreen, LOW);
      break;
    case 1:
      
      regWrite(northRed, LOW);
      regWrite(northYellow, HIGH);
      regWrite(northGreen, LOW);
      regWrite(southRed, LOW);
      regWrite(southYellow, HIGH);
      regWrite(southGreen, LOW);
      regWrite(westRed, HIGH);
      regWrite(westYellow, LOW);
      regWrite(westGreen, LOW);
      regWrite(eastRed, HIGH);
      regWrite(eastYellow, LOW);
      regWrite(eastGreen, LOW);
      break;
    case 2:
      
      regWrite(northRed, HIGH);
      regWrite(northYellow, LOW);
      regWrite(northGreen, LOW);
      regWrite(southRed, HIGH);
      regWrite(southYellow, LOW);
      regWrite(southGreen, LOW);
      regWrite(westRed, HIGH);
      regWrite(westYellow, HIGH);
      regWrite(westGreen, LOW);
      regWrite(eastRed, HIGH);
      regWrite(eastYellow, HIGH);
      regWrite(eastGreen, LOW);
      break;
    case 3:
      
      regWrite(northRed, HIGH);
      regWrite(northYellow, LOW);
      regWrite(northGreen, LOW);
      regWrite(southRed, HIGH);
      regWrite(southYellow, LOW);
      regWrite(southGreen, LOW);
      regWrite(westRed, LOW);
      regWrite(westYellow, LOW);
      regWrite(westGreen, HIGH);
      regWrite(eastRed, LOW);
      regWrite(eastYellow, LOW);
      regWrite(eastGreen, HIGH);
      break;
    case 4:
      regWrite( northRed, HIGH);
      regWrite( northYellow, LOW);
      regWrite( northGreen, LOW);
      regWrite( southRed, HIGH);
      regWrite( southYellow, LOW);
      regWrite( southGreen, LOW);
      regWrite( westRed, LOW);
      regWrite( westYellow, HIGH);
      regWrite( westGreen, LOW);
      regWrite( eastRed, LOW);
      regWrite( eastYellow, HIGH);
      regWrite( eastGreen, LOW);

      break;
    case 5:
      regWrite( northRed, HIGH);
      regWrite( northYellow, HIGH);
      regWrite( northGreen, LOW);
      regWrite( southRed, HIGH);
      regWrite( southYellow, HIGH);
      regWrite( southGreen, LOW);
      regWrite( westRed, HIGH);
      regWrite( westYellow, LOW);
      regWrite( westGreen, LOW);
      regWrite( eastRed, HIGH);
      regWrite( eastYellow, LOW);
      regWrite( eastGreen, LOW);
      break;
  }
}

void regWrite(int pin, bool state) {
  
  int reg = pin / 8;
  
  int actualPin = pin - (8 * reg);

  
  digitalWrite(latchPin, LOW);

  for (int i = 0; i < numOfRegisters; i++) {
  
    byte* states = &registerState[i];

   
    if (i == reg) {
      bitWrite(*states, actualPin, state);
    }

    
    shiftOut(dataPin, clockPin, MSBFIRST, *states);
  }


  digitalWrite(latchPin, HIGH);
}

boolean debounceSensor1(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[0]);
  if (state != stateNow)
  {

    stateNow = digitalRead(sensorPin[0]);
  }
  return stateNow;

}

boolean debounceSensor2(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[1]);
  if (state != stateNow)
  {
 
    stateNow = digitalRead(sensorPin[1]);
  }
  return stateNow;

}

boolean debounceSensor3(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[2]);
  if (state != stateNow)
  {
   
    stateNow = digitalRead(sensorPin[2]);
  }
  return stateNow;

}

boolean debounceSensor5(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[4]);
  if (state != stateNow)
  {
   
    stateNow = digitalRead(sensorPin[4]);
  }
  return stateNow;

}

boolean debounceSensor6(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[5]);
  if (state != stateNow)
  {
   
    stateNow = digitalRead(sensorPin[5]);
  }
  return stateNow;

}

boolean debounceSensor7(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[6]);
  if (state != stateNow)
  {
 
    stateNow = digitalRead(sensorPin[6]);
  }
  return stateNow;

}

boolean debounceSensor8(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[7]);
  if (state != stateNow)
  {
  
    stateNow = digitalRead(sensorPin[7]);
  }
  return stateNow;

}

boolean debounceSensor9(boolean state)
{
  boolean stateNow = digitalRead(sensorPin[8]);
  if (state != stateNow)
  {
  
    stateNow = digitalRead(sensorPin[8]);
  }
  return stateNow;

}
