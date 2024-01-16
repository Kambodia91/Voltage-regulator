//----------------------------BIBLIOTEKI----------------------------//
#include <Arduino.h>                                                // [Arduino]
#include <PIDController.h>
#include <ArduinoLogger.h>                                          // [Serial / Terminal]
#include "user.h"    
//-----------------------------Objects------------------------------//

PIDController pid;                                                  // Create an instance of the PID controller class, called "pid"

//------------------------------Define------------------------------//
int16_t               altVoltage      = (400 * BAT_CELLS * ALT_CALIB_ADC) / ALT_CALIB_REAL_VOLTAGE;
static int32_t        altVoltageFixdt = (400 * BAT_CELLS * ALT_CALIB_ADC) / ALT_CALIB_REAL_VOLTAGE << 16;  // Fixed-point filter output initialized at 400 V*100/cell = 4 V/cell converted to fixed-point
int16_t               voltageCalib;
extern int            outputCompiute;
int16_t               output;  

// Zmienna z blynk.cpp
int adcAltValue; 
int setPoint;

//------------------------------------------------------------------//

void filtLowPass32(int32_t u, uint16_t coef, int32_t *y) {
  int64_t tmp;  
  tmp = ((int64_t)((u << 4) - (*y >> 12)) * coef) >> 4;
  tmp = CLAMP(tmp, -2147483648LL, 2147483647LL);                    // Overflow protection: 2147483647LL = 2^31 - 1
  *y = (int32_t)tmp + (*y);
}

//------------------------------------------------------------------//

void compiutePID() {
  pid.setpoint(setPoint);                                           // The "goal" the PID controller tries to "reach"
  outputCompiute = pid.compute(voltageCalib);                       // Let the PID compute the value, returns the optimal output
}

//------------------------------------------------------------------//

void userpreSetup() {                                               // void Pre Setup.
  inf << "[MAIN] Implementing user Pre setup" << endl;
  
  pid.begin();                                                      // initialize the PID instance
  pid.tune(PID_KP, PID_KI, PID_KD);                                 // Tune the PID, arguments: kP, kI, kD
  pid.limit(PID_LIMIT_MIN, PID_LIMIT_MAX);                          // Limit the PID output between 0 and 255, this is important to get rid of integral windup!
  
  inf << "[MAIN] User Pre setup done" << endl;
}
//---------------------------VOID SETUP-----------------------------//

void userpostSetup() {                                              // void Setup.
  inf << "[MAIN] Implementing user Post setup" << endl;
  //
  inf << "[MAIN] User Post setup done" << endl;
}

//----------------------------VOID LOOP-----------------------------//

void userLoop() {                                                   // void Loop.
  unsigned long timeNow = millis();
  //digitalWrite(LED_BUILTIN, (timeNow%2000)<1000);
  
  //adcAltValue = analogRead(Esp_Input);                              // Read the value
  
  filtLowPass32(adcAltValue, ALT_FILT_COEF, &altVoltageFixdt);
  altVoltage = (int16_t)(altVoltageFixdt >> 16);                    // convert fixed-point to integer
  voltageCalib = altVoltage * ALT_CALIB_REAL_VOLTAGE / ALT_CALIB_ADC;
 
  compiutePID();

  ledcWrite(0, outputCompiute);                                     // Write the output to the output pin

}
//--------------------------END USER FILE---------------------------//