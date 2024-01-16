#ifndef VOLTAGEREGULATOR_H
#define VOLTAGEREGULATOR_H

#include <Arduino.h>

void filtLowPass32(int32_t u, uint16_t coef, int32_t *y);
void VoltageRegulatorInit();
void VoltageRegulatorCompiute();

#endif