
#include <Servo.h>

//driver consts
const int leftPlusPin = 9;
const int leftMinusPin = 6;
const int rightPlusPin = 10;
const int rightMinusPin = 11;

const int headingPin = 3;
const int pitchPin = 5;

const int pitchCal = 0;
const int headingCal = -10;

const float deadZoneHeading = 0.05;
const float deadZoneAcceleration = 0.05;

//remote control consts
const int PEAKS_TO_COUNT = 8;
const int DATA_BYTES = 4;
const int referenceFreq = 1500;

Servo headingServo;
Servo pitchServo;



void setup()
{
  
  #ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #endif
  #ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
  #endif 

//011
//100
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  
  
  Serial.begin(9600);
  pinMode(A0, INPUT);
  
  pinMode(leftPlusPin, OUTPUT);  
  pinMode(leftMinusPin, OUTPUT);  
  pinMode(rightPlusPin, OUTPUT);  
  pinMode(rightMinusPin, OUTPUT);
  pinMode(headingPin, OUTPUT);  
  pinMode(pitchPin, OUTPUT);  
  
  headingServo.attach(headingPin);
  pitchServo.attach(pitchPin);

  
}


int absMax = 0;


int vals[3];
int valIndex;

long zeroTimestamp = 0;
int waveFrontRegistered = 0;
int waveTailRegistered = 0;
long lastFreqChangeTimestamp = 0;


       /* #1 A bit about frequency reading
        ---------
          Assume we have frequency 3000Hz that changes to 4000Hz eventually. Let observe five sequentual readings of frequency.
          Reading one: 7 peaks in 2.33 msec. It means frequency of 3000Hz and it's ok. It is added to the pool
          Reading two: 7 peaks in 2.35 msec. It means frequency of 2978Hz and it's ok. There is no significant change, so it is 
                        added to the pool and we're going to the next reading
          Reading three: 7 peaks in 2.0 msec. It means frequency of 3500Hz. It's significant change, but we know, that it is not 
                        the old frequency, but it's not a new freqency as well, because it contains several peaks from the old
                        signal and several peaks from the new one. So it should be ignored, and only the next reading should be
                        interpreted as real frequency. So: by default all changes are ignored, but we set [shouldUseNextChange] to 1,
                        what means that the next reading won't be ignored and will be used as the next frequency. 
          Reading four: 7 peaks in 1.73 msec. It means 4046Hz and it's ok. It's the first reading in the new byte sequence. 
          Reading five: 7 peaks in 1.75msec, it mean 4000Hz and it's ok. Old freq ~= new freq, so new reading is not initiated.
                        Data is stored into average-calculations-pool and we're proceeding to the next measure.
        */        
int shouldUseNextChange = 0;


int r = 0;
int counter = 0;

int prevFreq = 0;

int measureLock = 1; //if measureLock is set, reading is ignored, but the next reading is treated as normal. used to skip half-right or delayed readings
int bytes[DATA_BYTES];
int byteIndex = 0;

int freqIndex = -1; 
int freqHistory[3] = {0,0,0};

inline void recordFreq(long freq)
{
            if (byteIndex < DATA_BYTES) {
              int freqZero = 2000; //range for byte 1 and byte 3 is 2000-3000Hz
              if (byteIndex%2 == 0) freqZero = 3500; //range for byte 0 and byte 2 is 3500-4500Hz
              bytes[byteIndex] = 256 * (freq - freqZero) / 1000;
/*              Serial.print("recorded: 256 * (");
              Serial.print(freq);
              Serial.print(" - ");
              Serial.print(freqZero);
              Serial.print(") / 1000 = ");
              Serial.println(bytes[byteIndex]);*/
              byteIndex ++;
            }
            lastFreqChangeTimestamp = micros();
}

inline boolean freqenciesAreClose(int freq1, int freq2) //true if they are closer than 100 Hz, false if not
{
  return abs(freq2 - freq1) < 250;
}

inline void rewriteMachineryStates(int xValue, int yValue, int hValue, int pValue)
{
      float heading = (xValue - 128.0) / 256.0;
      float acc = - (yValue - 128.0) / 256.0;
      
      float leftAcc = 0;
      float rightAcc = 0;
      
      if (acc > -deadZoneAcceleration && acc < deadZoneAcceleration) acc = 0;
    
      if (heading < -deadZoneHeading) {
        Serial.print(" -x<01 ");
        leftAcc = acc;
        rightAcc = acc * cos(heading * PI * 2);
      }
      if (heading > deadZoneHeading) {
        Serial.print(" -x>01 ");
        leftAcc = acc * cos(heading * PI * 2);
        rightAcc = acc;
      }
      if (heading > -deadZoneHeading && heading < deadZoneHeading) {
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
      
      if (pValue > 0 && pValue < 255) {
        float pv = pitchCal + 90 + (pValue - 128) * 90.0 / 128;
        if (pv < 0) pv = 0;
        if (pv > 180) pv = 180;
        pitchServo.write(pv);
        Serial.print("pitch: ");
        Serial.println(pv);
      }
      
      if (hValue > 0 && hValue < 240) {
        float hv = headingCal + 90 + (hValue - 128) * 90.0 / 128;
        if (hv < 0) hv = 0;
        if (hv > 180) hv = 180;
        headingServo.write(hv);
        Serial.print("heading: ");
        Serial.print(hv);
      }
            
      
}

void loop()
{
  int val = analogRead(A0);
  long cTime = micros();
  if (lastFreqChangeTimestamp == 0) lastFreqChangeTimestamp = cTime;
  
  vals[valIndex] = val;
  valIndex++;
  if (valIndex == 3) valIndex = 0;
  
  if (waveFrontRegistered == 0 && vals[0] != 0 && vals[1] != 0 && vals[2] != 0) {
    waveFrontRegistered = 1;
  }

  if (waveFrontRegistered == 1 && waveTailRegistered == 0 && vals[0] == 0 && vals[1] == 0 && vals[2] == 0) {
    waveTailRegistered = 1;
  }
  
  
  if (waveTailRegistered && waveFrontRegistered)
  {
    if (measureLock == 0) {
      if (counter == 0) {
        zeroTimestamp = cTime;
      }
      counter ++;
    } else {
      measureLock = 0;
    }
      
    waveTailRegistered = 0;
    waveFrontRegistered = 0;
      
    if (counter == PEAKS_TO_COUNT) {
      long interval = cTime - zeroTimestamp;
      long freq = (PEAKS_TO_COUNT - 1) * 1000000 / interval;
      measureLock = 1;
      counter = 0;
                      
      freqIndex ++;
      if (freqIndex == 3) freqIndex = 0;
      
      freqHistory[freqIndex] = freq;
      
      int freqIndex1 = freqIndex;
      int freqIndex2 = freqIndex - 1; if (freqIndex2 < 0) freqIndex2 += 3;
      int freqIndex3 = freqIndex - 2; if (freqIndex3 < 0) freqIndex3 += 3;
      
      
      if (freqenciesAreClose(freqHistory[freqIndex1], freqHistory[freqIndex2]) &&
          !freqenciesAreClose(freqHistory[freqIndex2], freqHistory[freqIndex3])) {
            
            if (abs(freq - referenceFreq) < 200) {
//              for (int i = 0; i < DATA_BYTES; i++ ){
//                Serial.print(bytes[i]);
//                Serial.print("; ");
//              }
//              Serial.println("");
              rewriteMachineryStates(bytes[0], bytes[1], bytes[2], bytes[3]);
              byteIndex = 0;
              lastFreqChangeTimestamp = micros();
            } else {
              recordFreq(freq);
            }
            
          }
            
      prevFreq = freq;
      
    } 
  }
}
