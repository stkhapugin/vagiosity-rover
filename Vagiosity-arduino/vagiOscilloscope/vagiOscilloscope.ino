int PIN_IN = 7;

volatile int counter = 0;

void interrupt()
{
  counter ++;
}


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
  delay(2000);
}


const int maxelems = 300;
byte values[maxelems];
int times[maxelems];
int index = 0;

long starttime = 0;

void loop()
{
  int val = analogRead(A0);
  unsigned long cTime = micros();
  if (starttime == 0) starttime = cTime;

/////////////////////////////////////////////        
        if (index < maxelems) {
          index ++;
          values[index] = val >> 2;
          times[index] = (int)(cTime - starttime);
          if (index == maxelems - 1) {
            for (int i = 0; i < maxelems; i ++) {
              Serial.print(times[i]);
              Serial.print("\t");
              Serial.println(values[i]);
            }
            delay(10000);
          }
        }
/////////////////////////////////////////////
    
}
