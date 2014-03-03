int PIN_IN = 7;

volatile int counter = 0;

void interrupt()
{
//  Serial.println("XXX");
  counter ++;
}

long timeStart = 0;
long TIME_SPAN = 10000; //span of frequency analysis span
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
  
  #ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #endif
  #ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
  #endif 

  cbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);
  
  
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(2, INPUT);
  pinMode(7, INPUT);
  //attachInterrupt(0, interrupt, CHANGE);
  timeStart = micros();
}

int BASE_MIN = 5;
int BASE_MAX = 50;
int prints = 0;
int maxval = BASE_MAX;
int minval = BASE_MIN;
int PEAKS_TO_COUNT = 7;

int absMax = 0;

int data[100];
long time[100];
int dataCount = 110;

int vals[3];
int valIndex;

int zeroLevel = 0;
long long zeroTimestamp = 0;

//int i = 0;
int r = 0;
void loop()
{
  int val = analogRead(A0);
  
/*  r++;
  if (r == 10000) {
    r = 0;
    Serial.println(val);
  }*/
  
  long cTime = micros();
  
  
  
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
