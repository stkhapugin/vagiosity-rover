int PIN_IN = 7;

volatile int counter = 0;

void interrupt()
{
  counter ++;
}

int timeStart = 0;
int TIME_SPAN = 1000; //span of frequency analysis span
int DELAY_BETWEEN_BYTES = 10000; //actual delay in 10000mksec, but we'll take 7500 to compensate possible delays
int referenceFreq = 1000;
int prevFreq = 0;

int currentFreqPool = 0; //pool for readings to calculate average frequency after reading is finished
int currentFreqPoolCount = 0; //count of pool elements

int bytePosition = 0; //shift from sychrofrequency

int byteMeasureStartTime = 0;

int averageReadFreq = 0;

int syncStarted = 0; //marks if we're reading syncrofreqency again after another completed bytes read
int nextReadingShouldBeNextByte = 0; //marks if we've detected significant freq change OR if enough time has passed

void setup()
{
  
    // defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// set prescale to 16
sbi(ADCSRA,ADPS2) ;
cbi(ADCSRA,ADPS1) ;
cbi(ADCSRA,ADPS0) ;


  Serial.begin(9600);
  pinMode(PIN_IN, INPUT);
  attachInterrupt(0, interrupt, RISING);
  timeStart = micros();
}

int bytes[3];
void processFreq(int freq, int bytePos)
{
  if (bytePos < 2) {
    bytes[bytePos] = freq;
  }
  
  if (bytePos == 2) {
    Serial.print("A: ");
    Serial.print(bytes[0]);
    Serial.print("B: ");
    Serial.print(bytes[1]);
    Serial.print("C: ");
    Serial.println(bytes[2]);
  }
  
}

void loop()
{
  //calculating frequency
    int val = analogRead(A0);
  unsigned long cTime = micros();
  
  vals[valIndex] = val;
  valIndex++;
  if (valIndex == 3) valIndex = 0;
  
  if (zeroLevel == 0 && vals[0] == 0 && vals[1] == 0 && vals[2] == 0) {
    zeroLevel = 1;
  }
  
  if (zeroLevel) {
    if (vals[0] != 0 && vals[1] != 0 && vals[2] != 0) {
      zeroLevel = 0;
      counter ++;
      if (counter == PEAKS_TO_COUNT) {
        int interval = cTime - zeroTimestamp;
        int freq = PEAKS_TO_COUNT * 1000000 / interval;
      
        //============================
        //here we got frequency calculated, so we need to use it wisely
        if (freq > referenceFreq - 100 && freq < referenceFreq + 100) {
          syncStarted = 1;
          nextReadingShouldBeNextByte = 0;
          currentFreqPool = 0; //clear pools
          currentFreqPoolCount = 0;
          bytePosition = 0;
          byteMeasureStartTime = cTime;
          if (prevFreq == 0) prevFreq = freq;
        }
        
        if (nextReadingShouldBeNextByte) {
          nextReadingShouldBeNextByte = 0;
          currentFreqPool = 0; //clear pools
          currentFreqPoolCount = 0;
          byteMeasureStartTime = cTime;
          bytePosition ++;
        }
    
        if (abs(freq - prevFreq) > 100 || cTime - byteMeasureStartTime > DELAY_BETWEEN_BYTES) { //significant frequesny change detected OR significant time span elapsed
          nextReadingShouldBeNextByte = 1;
          if (currentFreqPoolCount != 0) {
            averageReadFreq = currentFreqPool / currentFreqPoolCount;
          } else {
            averageReadFreq = 0;
          }
          processFreq(averageReadFreq, bytePosition);
        } else {
          currentFreqPool += freq;
          currentFreqPoolCount++;
        }
      
        //============================
      
        prints++;
        if (prints == 10) {
          Serial.print("Freq: ");
          Serial.println(freq);
          prints = 0;
        }
        
        counter = 0;
        zeroTimestamp = cTime;
      }
    }
  }
}
