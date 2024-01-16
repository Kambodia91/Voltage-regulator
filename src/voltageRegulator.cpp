/*

Voltage Regulator for alternator Denso 120A model.
The alternator has been modified to obtain a voltage of 42V.
Original connection diagram:
Modified connection diagram:

The regulator starts when the engine speed reaches 2000 Rpm.

*/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------ 

#include <PIDController.h>

#include "defines.h"
#include "config.h"
#include "setup.h"
#include "voltageRegulator.h"

//------------------------------------------------------------------------
// objects
//------------------------------------------------------------------------ 

PIDController pid;  // Create an instance of the PID controller class, called "pid"

//------------------------------------------------------------------------
// external variables
//------------------------------------------------------------------------ 
int BLYNK_SET_POINT;
int BLYNK_PID_KP;
int BLYNK_PID_KI;
int BLYNK_PID_KD;

//------------------------------------------------------------------------
// variables
//------------------------------------------------------------------------ 
int16_t               altVoltage      = (400 * BAT_CELLS * ALT_CALIB_ADC) / ALT_CALIB_REAL_VOLTAGE;
static int32_t        altVoltageFixdt = (400 * BAT_CELLS * ALT_CALIB_ADC) / ALT_CALIB_REAL_VOLTAGE << 16;  // Fixed-point filter output initialized at 400 V*100/cell = 4 V/cell converted to fixed-point
uint32_t       adcAltValue;                                        // [ADC IN] Actual input signal    
int16_t        voltageCalib;                                       // [V] Actual Voltage level.
int16_t               output;                                             // [PWM] Output Pwm to mosfet.

//------------------------------------------------------------------------
// procedures filtr low pas 32 bit
//------------------------------------------------------------------------ 
void filtLowPass32(int32_t u, uint16_t coef, int32_t *y) {
  int64_t tmp;  
  tmp = ((int64_t)((u << 4) - (*y >> 12)) * coef) >> 4;
  tmp = CLAMP(tmp, -2147483648LL, 2147483647LL);  // Overflow protection: 2147483647LL = 2^31 - 1
  *y = (int32_t)tmp + (*y);
}

//------------------------------------------------------------------------
// procedures voltage regulator initialize
//------------------------------------------------------------------------ 
void VoltageRegulatorInit() {
    pid.begin();          // initialize the PID instance
    pid.setpoint(PID_SET_POINT);    // The "goal" the PID controller tries to "reach"
    pid.tune(PID_KP, PID_KI, PID_KD);    // Tune the PID, arguments: kP, kI, kD
    pid.limit(PID_LIMIT_MIN, PID_LIMIT_MAX);    // Limit the PID output between 0 and 255, this is important to get rid of integral windup!
}

//------------------------------------------------------------------------
// procedures voltage regulator compiute
//------------------------------------------------------------------------ 
void VoltageRegulatorCompiute() {
    adcAltValue = analogRead(SENSOR_PIN);  // Read the value from the sensor
    
    filtLowPass32(adcAltValue, ALT_FILT_COEF, &altVoltageFixdt);
    altVoltage = (int16_t)(altVoltageFixdt >> 16);  // convert fixed-point to integer
    voltageCalib = altVoltage * ALT_CALIB_REAL_VOLTAGE / ALT_CALIB_ADC;
    // For Test BEGIN
    pid.setpoint(BLYNK_SET_POINT);    // The "goal" the PID controller tries to "reach"
    pid.tune(BLYNK_PID_KP, BLYNK_PID_KI, BLYNK_PID_KD);    // Tune the PID, arguments: kP, kI, kD
    // For Test END
    int outputCompiute = pid.compute(voltageCalib);    // Let the PID compute the value, returns the optimal output
  
    output = outputCompiute; //map(outputCompiute, 0 , 1023, 1023 , 0);   // Revers

#if defined(VARIANT_ESP32_S2)
    ledcWrite(0, output);                     // Write the output to the output pin
#else
    digitalWrite(OUTPUT_PIN, output);
#endif
    Serial.println(output);
}