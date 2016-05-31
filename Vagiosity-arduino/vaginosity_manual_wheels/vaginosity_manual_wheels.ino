 
#include <stdarg.h>


int leftPlusPin = 9;
int leftMinusPin = 6;
int rightPlusPin = 3;
int rightMinusPin = 5;

int xPin = A1;
int yPin = A0;


void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(leftPlusPin, OUTPUT);  
  pinMode(leftMinusPin, OUTPUT);  
  pinMode(rightPlusPin, OUTPUT);  
  pinMode(rightMinusPin, OUTPUT);  
  
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  float xValue = analogRead(xPin);
  float yValue = analogRead(yPin);

  float heading = (xValue - 512.0) / 1024.0;
  float acc = (512.0 - yValue) / 1024.0;
  
  float leftAcc = 0;
  float rightAcc = 0;

  if (heading < -0.03) {
    leftAcc = acc;
    rightAcc = acc * cos(heading * PI * 2);
  } else
  if (heading > 0.03) {
    leftAcc = acc * cos(heading * PI * 2);
    rightAcc = acc;
  } else
  if (heading > -0.03 && heading < 0.03) {
    leftAcc = acc;
    rightAcc = acc;
  }
  
  if (leftAcc < 0) {
    analogWrite(leftMinusPin, 510.0 * (-leftAcc));
    analogWrite(leftPlusPin, 0);
  } else 
  if (leftAcc >= 0) {
    analogWrite(leftPlusPin, 510.0 * leftAcc);
    analogWrite(leftMinusPin, 0);
  }
  
  if (rightAcc < 0) {
    analogWrite(rightMinusPin, 510.0 * (-rightAcc));
    analogWrite(rightPlusPin, 0);
  } else
  if (rightAcc >= 0) {
    analogWrite(rightPlusPin, 510.0 * rightAcc);
    analogWrite(rightMinusPin, 0);
  }
  
  delay(10);

  Serial.print("X: "); Serial.print(xValue);  
  Serial.print(" Y: "); Serial.print(yValue);  
  Serial.print(" head: "); Serial.print(heading);  
  Serial.print(" acc: "); Serial.print(acc);  
  Serial.print(" left: "); Serial.print(512.0 * leftAcc);  
  Serial.print(" right: "); Serial.println(512.0 * rightAcc);  
  
}
