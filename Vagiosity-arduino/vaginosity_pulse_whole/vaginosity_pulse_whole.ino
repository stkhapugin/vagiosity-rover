 
#include <Servo.h>


const int leftPlusPin = 9;
const int leftMinusPin = 6;
const int rightPlusPin = 10;
const int rightMinusPin = 11;

const int headingPin = 3;
const int pitchPin = 5;

const int pitchCal = 0;
const int headingCal = -10;

const int leftInPin = 8;
const int rightInPin = 7;

const int ONE = 0;
const int ZERO = 1 - ONE;


const int buttonPin = 2;
byte vagina[500];
byte penis[500];
int vaginaCounter = 0;
int penisFlag = 0;

unsigned long readings[50];
int readingsCount = 0;



const int MAX_TIME_BETWEEN_BYTES = 10000;
const int MAX_TIME_BETWEEN_BITS = 1000; //interval between pulses are 180 usec, so if interval is > 300, then it's a new number

Servo headingServo;
Servo pitchServo;

int oldLeft = 0, oldRight = 0, oldoldLeft = 0;
unsigned long lastBitReadTimestamp = 0;
unsigned long silenceTimestamp = 0;
unsigned long inputNumber = 0;
int inputDigits = 0;

unsigned long interruptedInputNumber = 0;
int bufferEmptied = false;


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
  
  oldLeft = ZERO;
  oldoldLeft = ZERO;
  
//  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2

}
//33322222222221111111111000000000
//21098765432109876543210987654321
//DDDDXXXXXXXYYYYYYYHHHHHHHPPPPPPP

//every bit is read when wave front arrives. next read should be done only after 
//wave tail has passed. this flag tells whether did we encounter wave tail or not yet
boolean readyForNewBit = true;

void applyPenisBytesToClitoris(unsigned long int data)
{

    int yRawValue = (data >> 24);
    int xRawValue = (data << 8 >> 24);
    int hRawValue = (data << 16 >> 24);
    int pRawValue = (data << 24 >> 24);

    int yValue = yRawValue * 4;
    int xValue = xRawValue * 4;
    int hValue = hRawValue * 4;
    int pValue = pRawValue * 4;  
    
      float heading = (xValue - 512.0) / 1024.0;
/*      float acc = 0;
      if (yValue < 450) {
        acc = (yValue - 450.0) / 450.0;
      } else if (yValue > 574) {
        acc = (yValue - 574.0) / 450.0;
      }*/
      float acc = - (yValue - 512.0) / 512.0;
//      if (abs(acc) < 0.05) acc = 0;
      
      float leftAcc = 0;
      float rightAcc = 0;
      
    
      if (heading < -0.03) {
        //Serial.print(" -x<01 ");
        leftAcc = acc;
        rightAcc = acc * cos(heading * PI * 2);
      }
      if (heading > 0.03) {
        //Serial.print(" -x>01 ");
        leftAcc = acc * cos(heading * PI * 2);
        rightAcc = acc;
      }
      if (heading > -0.03 && heading < 0.03) {
        //Serial.print(" -xbtw ");
        leftAcc = acc;
        rightAcc = acc;
      }
      
     // leftAcc = 0;
     // rightAcc = 0;
      
      if (leftAcc < 0) {
        analogWrite(leftMinusPin, -255.0 * leftAcc);
        analogWrite(leftPlusPin, 0);
      } else {
        analogWrite(leftPlusPin, 255.0 * leftAcc);
        analogWrite(leftMinusPin, 0);
      }
      
      if (rightAcc < 0) {
        analogWrite(rightMinusPin, -255.0 * rightAcc);
        analogWrite(rightPlusPin, 0);
      } else {
        analogWrite(rightPlusPin, 255.0 * rightAcc);
        analogWrite(rightMinusPin, 0);
      }
      
      Serial.print("xVal: "); Serial.print(xRawValue, HEX);
      Serial.print(" yVal: "); Serial.print(yRawValue, HEX);
      Serial.print(" pVal: "); Serial.print(pRawValue, HEX);
      Serial.print(" hVal: "); Serial.print(hRawValue, HEX);
      Serial.print(" --- head: "); Serial.print(heading);  
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
  
  int buttonValue = digitalRead(buttonPin);
  if (buttonValue) {
    penisFlag = 1;
    vaginaCounter = 0;
  }
  
  if (penisFlag) {
    if (vaginaCounter < 500) {
      vagina[vaginaCounter] = left;
      penis[vaginaCounter] = right;
      vaginaCounter++;
    }
    
    if (vaginaCounter == 500) {
      for (int i = 0; i < 500; i ++) {
        Serial.print(vagina[i]);
        Serial.print(" ");
        Serial.println(penis[i]);
      }
      vaginaCounter = 501;
    }
  }
  
  ///-------------
    unsigned long time = micros();
  if (left == ONE && oldLeft == ZERO && readyForNewBit == true) { //wave front encountered on SYNC channel
    readyForNewBit = false;
    if (time - lastBitReadTimestamp > MAX_TIME_BETWEEN_BITS && inputDigits > 0) {//inputDigits > 0 means than only existing number can be interrupted
      interruptedInputNumber = inputNumber << (32 - inputDigits);
      inputNumber = 0;
      inputDigits = 0;
    }
    inputNumber <<= 1;
    inputNumber += (right == ONE);
    inputDigits ++;
    lastBitReadTimestamp = time;
    
    if (inputDigits == 32) {
      if (interruptedInputNumber) {
//        Serial.print("previous number read, but interrupted: ");
//        Serial.println(interruptedInputNumber, HEX);
        interruptedInputNumber = 0;
      }
//      Serial.print("number read: ");
//      Serial.println(inputNumber, HEX);
      
      readings[readingsCount] = inputNumber;
      if (readingsCount < 49) readingsCount ++;
      
      applyPenisBytesToClitoris(inputNumber);
      
      inputDigits = 0;
      inputNumber = 0;
    }
  } else if (left == ZERO && oldLeft == ZERO && oldoldLeft == ZERO && !readyForNewBit) { //wave tail encountered
    readyForNewBit = true;
  } 
  
  oldoldLeft = oldLeft;
  oldLeft = left;
  
  //wait for silece for 100 msecs
  if (left == ONE) { //each HIGH reading ruins waiting
    silenceTimestamp = time;
    bufferEmptied = false;
  }
  if (time - silenceTimestamp > 100000 && !bufferEmptied) { //if there was only LOW readings for 100000 usecs, then print buffer contents and stored data
    readyForNewBit = true; //and reset all flags
    bufferEmptied = true;

      interruptedInputNumber = inputNumber << (32 - inputDigits);
      inputNumber = 0;
      inputDigits = 0;
       
      Serial.print("long pause, buffer contents: ");
      Serial.println(interruptedInputNumber, HEX);
      interruptedInputNumber = 0;
      
      for (int i = 0; i < readingsCount; i++) {
        Serial.print("number read: ");
        Serial.println(readings[i], HEX);
      }
      readingsCount = 0;
  }
}
