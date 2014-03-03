 
#include <RCSwitch.h>
#include <Servo.h>


int leftPlusPin = 9;
int leftMinusPin = 6;
int rightPlusPin = 10;
int rightMinusPin = 11;

int headingPin = 3;
int pitchPin = 5;

int pitchCal = 0;
int headingCal = -10;

Servo headingServo;
Servo pitchServo;

RCSwitch mySwitch = RCSwitch();

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(leftPlusPin, OUTPUT);  
  pinMode(leftMinusPin, OUTPUT);  
  pinMode(rightPlusPin, OUTPUT);  
  pinMode(rightMinusPin, OUTPUT);
  pinMode(headingPin, OUTPUT);  
  pinMode(pitchPin, OUTPUT);  
  
  headingServo.attach(headingPin);
  pitchServo.attach(pitchPin);

  Serial.begin(9600);
  
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2

}
//33322222222221111111111000000000
//21098765432109876543210987654321
//DDDDXXXXXXXYYYYYYYHHHHHHHPPPPPPP

void loop() {
  
  if (mySwitch.available()) {
    
    unsigned long int data = mySwitch.getReceivedValue();
    Serial.print(data, HEX);

    int yValue = (data << 4 >> 25) * 8;
    int xValue = (data << 11 >> 25) * 8;
    int hValue = (data << 18 >> 25) * 8;
    int pValue = (data << 25 >> 25) * 8;
    
/*    Serial.print(" x :"); Serial.print(xValue);
    Serial.print(" y :"); Serial.print(yValue);
    Serial.print(" h :"); Serial.print(hValue);
    Serial.print(" p :"); Serial.println(pValue);*/
    
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

    mySwitch.resetAvailable();
  }
  
}
