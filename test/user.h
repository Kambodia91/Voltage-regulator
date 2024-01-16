#include <Arduino.h>
#include "defines.h"

void filtLowPass32(int32_t u, uint16_t coef, int32_t *y);
void compiutePID();
void userpreSetup();
void userpostSetup();
void userLoop();