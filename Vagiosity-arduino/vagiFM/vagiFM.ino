int PIN_IN = 7;

volatile int counter = 0;

void interrupt()
{
  counter ++;
}

int timeStart = 0;
unsigned long DELAY_BETWEEN_BYTES = 250000; //actual delay in 10000mksec, but we'll take 7500 to compensate possible delays
int referenceFreq = 2000;
int prevFreq = 0;

unsigned long currentFreqPool = 0; //pool for readings to calculate average frequency after reading is finished
int currentFreqPoolCount = 0; //count of pool elements

int bytePosition = 0; //shift from sychrofrequency

unsigned long averageReadFreq = 0;

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

// set prescale
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);


  Serial.begin(9600);
  pinMode(PIN_IN, INPUT);
  attachInterrupt(0, interrupt, RISING);
  timeStart = micros();
}

const int BYTES_NUMBER = 4; //bytes count without sychroimpulse

int bytes[BYTES_NUMBER];
int freqs[BYTES_NUMBER];
unsigned long times[BYTES_NUMBER];
unsigned long timeZero = 0;

//returns 1 when buffer overflown (when all bytes are set)
int processFreq(int freq, int bytePos)
{
  if (bytePos < BYTES_NUMBER) {
    bytes[bytePos] = (freq - 2500) * 256 / 2000;
    freqs[bytePos] = freq;
    times[bytePos] = micros() - timeZero;
  }
  
  if (bytePos == BYTES_NUMBER - 1) {
    Serial.print("A: ");
    Serial.print(bytes[0]);
    Serial.print(" (");
    Serial.print(freqs[0]);
    Serial.print(" at ");
    Serial.print(times[0]);
    
    Serial.print("), B: ");
    Serial.print(bytes[1]);
    Serial.print(" (");
    Serial.print(freqs[1]);
    Serial.print(" at ");
    Serial.print(times[1]);
    
    Serial.print("), C: ");
    Serial.print(bytes[2]);
    Serial.print(" (");
    Serial.print(freqs[2]);
    Serial.print(" at ");
    Serial.print(times[2]);
    
    Serial.print("), D: ");
    Serial.print(bytes[3]);
    Serial.print(" (");
    Serial.print(freqs[3]);
    Serial.print(" at ");
    Serial.print(times[3]);
    
    Serial.println(")");
    return 1;
    
  }
  
  return 0;
}

int prints = 0;
unsigned long PEAKS_TO_COUNT = 7;


int vals[3];
int valIndex;

int zeroLevel = 0;
unsigned long zeroTimestamp = 0;
unsigned long byteMeasureStartTime = 0;


//int i = 0;
int r = 0;


const int imax = 10;
unsigned long timestamps[imax];
int datas[imax];
byte events[imax];
int index = imax+2;

void loop()
{
  //calculating frequency
    int val = analogRead(A0);
  unsigned long cTime = micros();
  
  vals[valIndex] = val;
  valIndex++;
  if (valIndex == 3) valIndex = 0;
  
  //if there wasn't zerolevel, and we have three zeros in a row, then set [zerolevel] flag
  //actually we're catching wave tail here
  if (zeroLevel == 0 && vals[0] == 0 && vals[1] == 0 && vals[2] == 0) {
    zeroLevel = 1;
  }
  
  if (zeroLevel) { //if wave tail was catched
    //let's check if we have wave front coming
    if (vals[0] != 0 && vals[1] != 0 && vals[2] != 0) {
      //if yes, it means that we've just catched wave tail followed by wave front,
      // this means we observed wave period
      zeroLevel = 0; //clear zerolevel flag, so we can catch wave tail again 
      counter ++; //increment period counter
      if (counter == PEAKS_TO_COUNT) { //if we've counred enough peaks,
        long interval = cTime - zeroTimestamp; //calc time interval
        long freq = PEAKS_TO_COUNT * 1000000 / interval; //and frequency
        if (freq < 400) {
          Serial.print("bad freq ");
          Serial.print(freq);
          Serial.print(" = 7 * 1000000 / ");
          Serial.println(interval);
        }
        

/////////////////////////////////////////////
/*        if (index == imax+2) index = 0;
        
        if (index < imax) {
          index ++;
          datas[index] = freq;
          times[index] = cTime;
          events[index] = 0;
          if (index == imax - 1) {
            for (int i = 0; i < imax; i ++) {
              Serial.print(times[i]);
              Serial.print("\t");
              Serial.print(datas[i]);
              Serial.print("\t");
              Serial.println(events[i], BIN);
            }
            delay(10000);
          }
        }*/
/////////////////////////////////////////////
      
        //============================
        //here we got frequency calculated, so we need to use it wisely
        if (!syncStarted && freq > referenceFreq - 100 && freq < referenceFreq + 100) {
          //if we're close enough to the reference frequency
          syncStarted = 1; //let program know that synchronization was started
          nextReadingShouldBeNextByte = 0; //next reading is not next byte yet. wait for significant frequency change in order to move to the next byte.
          currentFreqPool = 0; //clear average frequency calculating pool
          currentFreqPoolCount = 0; //anc count for that pool
          bytePosition = -1; //reset byte sequence position counter. -1 means than the NEXT different frequency will be 0th byte in sequence
          byteMeasureStartTime = cTime; //remember synchroimpulse starting time
          prevFreq = freq; //store frequency
          timeZero = cTime;
          
          if (index < imax) events[index] |= 1;
        }
        
        /* #1 A bit about frequency reading
        ---------
          Assume we have frequency 3000Hz that changes to 4000Hz eventually. Let observe five sequentual readings of frequency.
          Reading one: 7 peaks in 2.33 msec. It means frequency of 3000Hz and it's ok. It is added to the pool
          Reading two: 7 peaks in 2.35 msec. It means frequency of 2978Hz and it's ok. There is no significant change, so it is 
                        added to the pool and we're going to the next reading
          Reading three: 7 peaks in 2.0 msec. It means frequency of 3500Hz. It's significant change, but we know, that it is not 
                        the old frequency, but it's not a new freqency as well, because it contains several peaks from the old
                        signal and several peaks from the new one. So it should be ignored, and only the next reading should be
                        interpreted as real frequency. So: [nextReadingShouldBeNextByte] is set to 1. And because we set this flag
                        _after_ we check for it, we'll check and register byte index change only on the next loop iteration.
          Reading four: 7 peaks in 1.73 msec. It means 4046Hz and it's ok. It's the first reading in the new byte sequence. 
                        BUT: we set prevFreq = freq right after reading to prevent is-it-time-for-the-next-byte-checking block
                        from execution on this cycle. Because, obviously, our new freqency (4046Hz) differs from the old one (3000Hz),
                        but this doesn't mean we need to initiate byte changing routines, because we've just changed the byte index.
          Reading five: 7 peaks in 1.75msec, it mean 4000Hz and it's ok. Old freq ~= new freq, so new reading is not initiated.
                        Data is stored into average-calculations-pool and we're proceeding to the next measure.
        
        */
        
        if (nextReadingShouldBeNextByte) { //if it's time to recognize next byte
          nextReadingShouldBeNextByte = 0; //drop this flag (because next byte presence is checked by another block of code)
          currentFreqPool = 0; //clear pools
          currentFreqPoolCount = 0;
          byteMeasureStartTime = cTime;
          bytePosition ++; //increment byte position
          prevFreq = freq; //to prevent the next block of being called this time (look #1)
          if (index < imax) events[index] |= 2;
        }
    
        //significant frequecny change detected OR significant time span elapsed
        //means it's time to record next byte
        //this should go AFTER checking [nextReadingShouldBeNextByte], look #1
        if (abs(freq - prevFreq) > 100 || cTime - byteMeasureStartTime > DELAY_BETWEEN_BYTES) {
          if (abs(freq - prevFreq) > 100) {
            if (index < imax) events[index] |= 4;
            Serial.print("next byte by measure: ");
            Serial.print(freq);
            Serial.print(" vs ");
            Serial.println(prevFreq);
          }
          
          if (cTime - byteMeasureStartTime > DELAY_BETWEEN_BYTES) {
            if (index < imax) events[index] |= 8;
            Serial.println("next byte by time");
          }
          
          nextReadingShouldBeNextByte = 1; //as I said, it's time to read next byte
          if (currentFreqPoolCount != 0) { //if we have some values in pool accumulated
            averageReadFreq = currentFreqPool / currentFreqPoolCount; //calc average frequency
          } else {
            averageReadFreq = freq; //or keep it equal to single reading that has just happened
          }
          
          if (bytePosition >= 0) {
            if (index < imax) events[index] |= 16;
            int result = processFreq(averageReadFreq, bytePosition); //process frequency
            if (result == 1) {
              if (index < imax) events[index] |= 32;
              syncStarted = 0;
            }
          }
          
        } else {
          currentFreqPool += freq; //if there is no changes and not too much time passed, just accumlate values
          currentFreqPoolCount++;
        }
      
        //============================
        
        counter = 0;
        zeroTimestamp = cTime;
        prevFreq = freq;
        
        prints++;
        if (prints == 10) {
          Serial.print("Freq: ");
          Serial.println(freq);
//          prints = 0;
        }
      }
    }
  }
}
