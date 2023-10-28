#include <FirebaseESP32.h>
#include  <WiFi.h>
#include <ESP32Servo.h>

#define FIREBASE_HOST "smart-traffic-managemanet-default-rtdb.firebaseio.com/"
#define WIFI_SSID "iPhone" // Change the name of your WIFI
#define WIFI_PASSWORD "12345678" // Change the password of your WIFI
#define FIREBASE_Authorization_key "dzAKLhiaW4tLtupQVDFR70FdCTA8et6NhhsX53i7"


FirebaseData firebaseData;
FirebaseJson json;

int lane0, lane1, lane2, lane3;
boolean emergency, gate;

int gateState = 0;

int buttonPin = 19;
boolean buttonState = LOW; 
boolean swState;

Servo myservo;

int pos = 0;
int servoPin = 5;
int sw = 0;
int buzzerPin = 18;

int openAngle = 90;
int closeAngle = 0;
int closeTime = 3000;

unsigned long previousMillis = 0;
const long interval = 3000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2400);

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(300);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_Authorization_key);
}

void loop() {
  // put your main code here, to run repeatedly:
  

  sw = digitalRead(buttonPin);
  if( sw == 1){
     test_sim800_module();
  send_SMS();
  Firebase.setFloat(firebaseData, "/Power/Status", sw);
    }
  else if ( sw == 0) {
 

  

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.println("Received data: " + data);

    


    sscanf(data.c_str(), "Lane0:%d,Lane1:%d,Lane2:%d,Lane3:%d,Emergency:%d,Gate:%d", &lane0, &lane1, &lane2, &lane3, &emergency, &gate);



  
  }


 

 

  Firebase.setFloat(firebaseData, "/Lane/Lane01", lane0);
  Firebase.setFloat(firebaseData, "/Lane/Lane02", lane1);
  Firebase.setFloat(firebaseData, "/Lane/Lane03", lane2);
  Firebase.setFloat(firebaseData, "/Lane/Lane04", lane3);

  Firebase.setFloat(firebaseData, "/Gate/Gate1", gate);
  Firebase.setFloat(firebaseData, "/Gate/Gate2", gate);



  Firebase.setFloat(firebaseData, "/Power/Status", sw);
  
  if (gate == 1 && gateState == 0) {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(300);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(300);
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(1500);

    for (int i = closeAngle; i <= openAngle; i++) {
      myservo.write(i);
      delay(5);
    }
    delay(closeTime);

    for (int i = openAngle; i >= closeAngle; i--) {
      myservo.write(i);
      delay(5);
    }

    
    gateState = 1;
  }
 
  else if (gate != 1 && gateState == 1) {
    
    gateState = 0;
  }


  }
}
boolean debounceButton(boolean state)
{
  boolean stateNow = digitalRead(buttonPin);
  if(state!=stateNow)
  {
 
    stateNow = digitalRead(buttonPin);
  }
  return stateNow;
  
}

void test_sim800_module()
{
  Serial2.println("AT");
  updateSerial();
  Serial.println();
  Serial2.println("AT+CSQ");
  updateSerial();
  Serial2.println("AT+CCID");
  updateSerial();
  Serial2.println("AT+CREG?");
  updateSerial();
  Serial2.println("ATI");
  updateSerial();
  Serial2.println("AT+CBC");
  updateSerial();
}
void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    Serial2.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())
  {
    Serial.write(Serial2.read());
  }
}
void send_SMS()
{
  Serial2.println("AT+CMGF=1"); 
  updateSerial();
  Serial2.println("AT+CMGS=\"+94713535964\"");
  Serial2.print("Power Failure Detected on JCT09X3C");
  updateSerial();
Serial.println();
  Serial.println("Message Sent");
  Serial2.write(26);
}
