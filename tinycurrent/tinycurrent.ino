// require emonlib from energymonitor.org
#include <EmonLib.h>
// require software serial port to support Atmel attiny85 microcontroller
#include <SoftwareSerial.h>

// attiny85 virtual serial port: Pin 5 RX, Pin 6 TX
SoftwareSerial oSerial(0, 1);

// Current Monitors
EnergyMonitor monPhase0;
EnergyMonitor monPhase1;

// iteration counter, loops since boot
long count = 0;

// initialize the board for operation
void setup()  
{
  oSerial.begin(9600);  // data rate for serial port
  
  // power factor correction = 200A (line current) / 0.1A (ct current) / 33 ohms (burden)
  float pfc = 60.606;
  monPhase0.current(2, pfc); // Phase 0 monitor
  monPhase1.current(3, pfc); // Phase 1 monitor
  
  delay (500);  // let circuit settle half a second
}

// run in a tight loop collecting data
// every 100ms transmit collected data via serial
void loop()
{
      count++ ;
      oSerial.print(count);
      //oSerial.print(",");
      //oSerial.print(getVcc());
      //oSerial.print(",");
      //oSerial.print(getTemp());
      oSerial.print(",");
      oSerial.print(analogRead(1));  // Unbuffered input
      oSerial.print(",");
      // Phase 0 RMS
      double rmsPhase0 = (monPhase0.calcIrms(1480) * 230.0);
      oSerial.print(rmsPhase0);
      oSerial.print(",");
      // Phase 1 RMS
      double rmsPhase1 = (monPhase1.calcIrms(1480) * 230.0);
      oSerial.print(rmsPhase1);
      oSerial.print("\n");
      delay(100);
}


long getVcc() {
  long result;
  // Read 1.1V reference against AVcc
  #if defined(__AVR_ATmega32U4__)
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADCSRB = _BV(MUX5); // the MUX5 bit is in the ADCSRB register
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(REFS1) | _BV(MUX5) | _BV(MUX1);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(REFS1) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
  #else
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  result = result; // add 1100 mV to get Vcc
  return result;
}

long readTemp() { 
  // Read 1.1V reference against AVcc
  #if defined(__AVR_ATmega32U4__)
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADCSRB = _BV(MUX5); // the MUX5 bit is in the ADCSRB register
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(REFS1) | _BV(MUX5) | _BV(MUX1);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(REFS1) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
  #else
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  #endif

  delay(2); // Wait for ADMUX setting to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low; // combine the two

  return result;
}


float getTemp() { 
  // replace these constants with your 2 data points
  // these are sample values that will get you in the ballpark (in degrees C)
  float temp1 = 0;
  long data1 = 274;
  float temp2 = 25.0;
  long data2 = 304;
  long rawData = readTemp();
 
  // calculate the scale factor
  float scaleFactor = (temp2 - temp1) / (data2 - data1);

  // now calculate the temperature
  float temp = scaleFactor * (rawData - data1) + temp1;

  return temp;
}
