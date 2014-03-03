 
#include <Servo.h>


int headingPin = 6;
int pitchPin = 5;

int xPin = A0;
int yPin = A1;

int pitchCal = 0;
int headingCal = -10;

Servo headingServo;
Servo pitchServo;

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(headingPin, OUTPUT);  
  pinMode(pitchPin, OUTPUT);  
  
  Serial.begin(9600);
  
  headingServo.attach(6);
  pitchServo.attach(5);
}

void loop() {
  float xValue = analogRead(xPin);
  float yValue = analogRead(yPin);

//  headingServo.write(90 + (xValue - 512) * 90 / 512);
  float pv = pitchCal + 90 + (xValue - 512) * 90.0 / 512.0;
  float hv = headingCal + 90 + (yValue - 512) * 90.0 / 512.0;
  if (hv < 0) hv = 0; if (pv < 0) pv = 0;
  if (hv > 180) hv = 180; if (pv > 180) pv = 180;
  pitchServo.write(pv);
  headingServo.write(hv);
//  headingServo.write(10 + (millis() - 2000) / 250);// + (xValue - 512) * 90 / 512);
//  pitchServo.write(90);// + (yValue - 512) * 90 / 512);
  
}
