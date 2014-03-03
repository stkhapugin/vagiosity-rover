 
#include <stdarg.h>


int leftPlusPin = 9;
int leftMinusPin = 6;
int rightPlusPin = 10;
int rightMinusPin = 11;

int xPin = A0;
int yPin = A1;


void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(leftPlusPin, OUTPUT);  
  pinMode(leftMinusPin, OUTPUT);  
  pinMode(rightPlusPin, OUTPUT);  
  pinMode(rightMinusPin, OUTPUT);  
  
  Serial.begin(9600);
}

void loop() {
  float xValue = analogRead(xPin);
  float yValue = analogRead(yPin);

  float heading = (xValue - 512.0) / 1024.0;
  float acc = (yValue - 512.0) / 1024.0;
  
  float leftAcc = 0;
  float rightAcc = 0;

  if (heading < -0.03) {
    Serial.print("-x<01 ");
    leftAcc = acc;
    rightAcc = acc * cos(heading * PI * 2);
  }
  if (heading > 0.03) {
    Serial.print("-x>01 ");
    leftAcc = acc * cos(heading * PI * 2);
    rightAcc = acc;
  }
  if (heading > -0.03 && heading < 0.03) {
    Serial.print("-xbtw ");
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
  
  delay(10);

  Serial.print("head: "); Serial.print(heading);  
  Serial.print(" acc: "); Serial.print(acc);  
  Serial.print(" left: "); Serial.print(leftAcc);  
  Serial.print(" right: "); Serial.println(rightAcc);  
  
}
