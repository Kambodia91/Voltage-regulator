//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------ 
#include <Arduino.h>
#include <ArduinoLogger.h>                         // [Serial / Terminal]
#include "defines.h"
#include "config.h"
#include "setup.h"
#include "voltageRegulator.h"

//------------------------------------------------------------------------
// main setup
//------------------------------------------------------------------------ 
void setup () {
  setupPlatform();  // installation of necessary things
  VoltageRegulatorInit(); // installation of a voltage regulator
}

//------------------------------------------------------------------------
// main loop
//------------------------------------------------------------------------ 
void loop () {
  loopPlatform();   // installation of necessary things
  VoltageRegulatorCompiute();
  
  //---Blink Led---//
  unsigned long timeNow = millis();
  digitalWrite(LED_BUILTIN, (timeNow%2000)<1000);
  //---Blink Led---//
}

//------------------------------------------------------------------------
// end files
//------------------------------------------------------------------------ 