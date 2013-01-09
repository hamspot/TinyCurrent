// require emonlib from energymonitor.org
#include <EmonLib.h>
// require software serial port to support Atmel attiny85 microcontroller
#include <SoftwareSerial.h>

/* Pin assignments based on chip used 8 pin vs 28 pin */
#if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
int PIN_TXD = 1;    // Pin 6
int PIN_RXD = 0;    // Pin 5
int PIN_PHASE0 = 3; // Pin 2
int PIN_PHASE1 = 4; // Pin 3
int PIN_VOLTS = 2;  // Pin 7
#else
int PIN_TXD = 1;    // Pin 3
int PIN_RXD = 0;    // Pin 2
int PIN_PHASE0 = A0; // Pin 23
int PIN_PHASE1 = A1; // Pin 24
int PIN_VOLTS = A2;  // Pin 25
#endif

/* serial port is software on attiny and hardware on atmega */
#if defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
// attiny85 virtual serial port: Pin 5 RX, Pin 6 TX
SoftwareSerial Serial(PIN_RXD, PIN_TXD);
#endif 

// Current Monitors
EnergyMonitor monPhase0;
EnergyMonitor monPhase1;

// iteration counter, loops since boot
long count = 0;

// initialize the board for operation
void setup()  
{
  // Apply clock calibration obtained from TinyTuner
  OSCCAL=0x4A;
  
  Serial.begin(19200);  // data rate for serial port
  
  Serial.println(";bootup TinyCurrent v0.1");
  
  // initialize the pins
  pinMode(PIN_VOLTS, INPUT_PULLUP);
  pinMode(PIN_PHASE0, INPUT_PULLUP);
  pinMode(PIN_PHASE1, INPUT_PULLUP);
  
  // power factor correction = 200A (line current) / 0.1A (ct current) / 33 ohms (burden)
  float pfc = 60.606;
  monPhase0.current(PIN_PHASE0, pfc); // Phase 0 monitor
  monPhase1.current(PIN_PHASE1, pfc); // Phase 1 monitor
  
  delay (500);  // let circuit settle half a second
}

// run in a tight loop collecting data
// every 100ms transmit collected data via serial
void loop()
{
      count++ ;
      Serial.print(count);
      Serial.print(",");
      Serial.print(analogRead(PIN_VOLTS));  // Unbuffered input
      Serial.print(",");
      // Phase 0 RMS
      double rmsPhase0 = (monPhase0.calcIrms(1480) * 230.0);
      Serial.print(rmsPhase0);
      Serial.print(",");
      // Phase 1 RMS
      double rmsPhase1 = (monPhase1.calcIrms(1480) * 230.0);
      Serial.print(rmsPhase1);
      Serial.print("\n");
      delay(100);
}

