
struct FreqRec;

struct FreqRec {
  int freq;
  struct FreqRec *prev;
};

struct FreqRec freqRecs[3];
FreqRec* currentFreq;


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
  pinMode(2, INPUT);
  pinMode(7, INPUT);

  freqRecs[2].prev = &freqRecs[1];
  freqRecs[1].prev = &freqRecs[0];
  freqRecs[0].prev = &freqRecs[2];

  currentFreq = &freqRecs[2];
}

const int PEAKS_TO_COUNT = 10;
const int DATA_BYTES = 4;
const int referenceFreq = 2000;
const long IMPULSE_LENGTH = 12500;

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

int syncHappened = 0;

void recordFreq(long freq)
{
  if (byteIndex < DATA_BYTES) {
    bytes[byteIndex] = 256 * (freq - 2500) / 1500;
    /*              Serial.print("recorded: 256 * (");
     Serial.print(freq);
     Serial.print(" - 2500) / 2000 = ");
     Serial.println(bytes[byteIndex]);*/
    byteIndex ++;
  }
  lastFreqChangeTimestamp = micros();
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
    } 
    else {
      measureLock = 0;
    }

    waveTailRegistered = 0;
    waveFrontRegistered = 0;

    if (counter == PEAKS_TO_COUNT) {
      long interval = cTime - zeroTimestamp;
      long freq = (PEAKS_TO_COUNT - 1) * 1000000 / interval;
      measureLock = 1;
      counter = 0;

      /*        prints++;
       if (prints == 100 || freq < 400) {
       Serial.print("Freq: ");
       Serial.println(freq);
       prints = 0;
       }
       */

      if (syncHappened == 0 && abs(freq - referenceFreq) < 100) {
        for (int i = 0; i < DATA_BYTES; i++ ){
          Serial.print(bytes[i]);
          Serial.print("; ");
        }
        Serial.println("");
        byteIndex = 0;
        lastFreqChangeTimestamp = micros();
        syncHappened = 1;
      }

      if (cTime - lastFreqChangeTimestamp > IMPULSE_LENGTH) {
        //        Serial.print("by time ");
        syncHappened = 0;
        recordFreq(freq);
      }

      prevFreq = freq;

    } 
  }
}

