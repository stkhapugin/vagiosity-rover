 
#include <Servo.h>


const int leftPlusPin = 9;
const int leftMinusPin = 6;
const int rightPlusPin = 10;
const int rightMinusPin = 11;

const int headingPin = 3;
const int pitchPin = 5;

const int pitchCal = 0;
const int headingCal = -10;

const int leftInPin = 7;
const int rightInPin = 8;

const int ONE = 1;
const int ZERO = 1 - ONE;

const int MAX_TIME_BETWEEN_BYTES = 10000;

Servo headingServo;
Servo pitchServo;



void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(leftPlusPin, OUTPUT);  
  pinMode(leftMinusPin, OUTPUT);  
  pinMode(rightPlusPin, OUTPUT);  
  pinMode(rightMinusPin, OUTPUT);
  pinMode(headingPin, OUTPUT);  
  pinMode(pitchPin, OUTPUT); 
 
  pinMode(leftInPin, INPUT_PULLUP);
  pinMode(rightInPin, INPUT_PULLUP);
  
  headingServo.attach(headingPin);
  pitchServo.attach(pitchPin);

  Serial.begin(9600);
  
//  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2

}
//33322222222221111111111000000000
//21098765432109876543210987654321
//DDDDXXXXXXXYYYYYYYHHHHHHHPPPPPPP

int oldLeft = 0, oldRight = 0;
unsigned long lastBitReadTimestamp = 0;
unsigned long inputNumber = 0;
int inputDigits = 0;

void applyData(unsigned long int data)
{

    int yValue = (data << 4 >> 25) * 8;
    int xValue = (data << 11 >> 25) * 8;
    int hValue = (data << 18 >> 25) * 8;
    int pValue = (data << 25 >> 25) * 8;  
    
      float heading = (xValue - 512.0) / 1024.0;
      float acc = - (yValue - 512.0) / 1024.0;
      
      float leftAcc = 0;
      float rightAcc = 0;
    
      if (heading < -0.03) {
        Serial.print(" -x<01 ");
        leftAcc = acc;
        rightAcc = acc * cos(heading * PI * 2);
      }
      if (heading > 0.03) {
        Serial.print(" -x>01 ");
        leftAcc = acc * cos(heading * PI * 2);
        rightAcc = acc;
      }
      if (heading > -0.03 && heading < 0.03) {
        Serial.print(" -xbtw ");
        leftAcc = acc;
        rightAcc = acc;
      }
      
      leftAcc = 0;
      rightAcc = 0;
      
      if (leftAcc < 0) {
        analogWrite(leftMinusPin, 255.0 * leftAcc);
        analogWrite(leftPlusPin, 0);
      } 
      if (leftAcc >= 0) {
        analogWrite(leftPlusPin, 255.0 * leftAcc);
        analogWrite(leftMinusPin, 0);
      }
      
      if (rightAcc < 0) {
        analogWrite(rightMinusPin, 255.0 * rightAcc);
        analogWrite(rightPlusPin, 0);
      }
      if (rightAcc >= 0) {
        analogWrite(rightPlusPin, 255.0 * rightAcc);
        analogWrite(rightMinusPin, 0);
      }
      
      Serial.print("head: "); Serial.print(heading);  
      Serial.print(" acc: "); Serial.print(acc);  
      Serial.print(" left: "); Serial.print(leftAcc);  
      Serial.print(" right: "); Serial.println(rightAcc); 
            
      float pv = pitchCal + 90 + (hValue - 512) * 90.0 / 512.0;
      float hv = headingCal + 90 + (pValue - 512) * 90.0 / 512.0;
      if (hv < 0) hv = 0; if (pv < 0) pv = 0;
      if (hv > 180) hv = 180; if (pv > 180) pv = 180;
      pitchServo.write(pv);
      headingServo.write(hv);
}

void loop() {
  
  int left = digitalRead(leftInPin);
  int right = digitalRead(rightInPin);
  
  if (left == ONE && oldLeft == ZERO) {
    unsigned long time = micros();
    if (time - lastBitReadTimestamp > MAX_TIME_BETWEEN_BYTES) {
      inputNumber = 0;
      inputDigits = 0;
    }
    inputNumber <<= 1;
    inputNumber += (left == ONE);
    inputDigits ++;
    lastBitReadTimestamp = micros();
    
    if (inputDigits == 32) {
      Serial.print("number read: ");
      Serial.println(inputNumber, HEX);
      
      applyData(inputNumber);
      
      inputDigits = 0;
      inputNumber = 0;
  }
  
  
  /*  if (mySwitch.available()) {
    
    unsigned long int data = mySwitch.getReceivedValue();
    Serial.print(data, HEX);

    int yValue = (data << 4 >> 25) * 8;
    int xValue = (data << 11 >> 25) * 8;
    int hValue = (data << 18 >> 25) * 8;
    int pValue = (data << 25 >> 25) * 8;
  */  
/*    
      float heading = (xValue - 512.0) / 1024.0;
      float acc = - (yValue - 512.0) / 1024.0;
      
      float leftAcc = 0;
      float rightAcc = 0;
    
      if (heading < -0.03) {
        Serial.print(" -x<01 ");
        leftAcc = acc;
        rightAcc = acc * cos(heading * PI * 2);
      }
      if (heading > 0.03) {
        Serial.print(" -x>01 ");
        leftAcc = acc * cos(heading * PI * 2);
        rightAcc = acc;
      }
      if (heading > -0.03 && heading < 0.03) {
        Serial.print(" -xbtw ");
        leftAcc = acc;
        rightAcc = acc;
      }
      
      leftAcc = 0;
      rightAcc = 0;
      
      if (leftAcc < 0) {
        analogWrite(leftMinusPin, 255.0 * leftAcc);
        analogWrite(leftPlusPin, 0);
      } 
      if (leftAcc >= 0) {
        analogWrite(leftPlusPin, 255.0 * leftAcc);
        analogWrite(leftMinusPin, 0);
      }
      
      if (rightAcc < 0) {
        analogWrite(rightMinusPin, 255.0 * rightAcc);
        analogWrite(rightPlusPin, 0);
      }
      if (rightAcc >= 0) {
        analogWrite(rightPlusPin, 255.0 * rightAcc);
        analogWrite(rightMinusPin, 0);
      }
      
      Serial.print("head: "); Serial.print(heading);  
      Serial.print(" acc: "); Serial.print(acc);  
      Serial.print(" left: "); Serial.print(leftAcc);  
      Serial.print(" right: "); Serial.println(rightAcc); 
            
      float pv = pitchCal + 90 + (hValue - 512) * 90.0 / 512.0;
      float hv = headingCal + 90 + (pValue - 512) * 90.0 / 512.0;
      if (hv < 0) hv = 0; if (pv < 0) pv = 0;
      if (hv > 180) hv = 180; if (pv > 180) pv = 180;
      pitchServo.write(pv);
      headingServo.write(hv);

      delay(10);

  }
  */
  
  
  
}
