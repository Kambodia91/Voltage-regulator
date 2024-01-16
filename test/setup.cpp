#include "defines.h"
#include "config.h"
#include "setup.h"
#include <ArduinoLogger.h>                         // [Serial / Terminal]
#include "blynk.h"

void UART_Init() {
  Serial.begin(SpeedRate);
  Serial.println();
}


void LOGGER_Init() {
  logger.add(Serial, LOG_LEVEL_INFO, true); // This will log everything on Serial
  // logger.add(Terminal, LOG_LEVEL_VERBOSE, true); // This will log everything on Serial
  // logger.disableLevelName(Terminal);
  
  inf << " " << endl;
  inf << "[MAIN] Hardware initialized, starting program load" << endl;
  inf << "[MAIN] Monitor_speed = " << SpeedRate << endl;
}

void GPIO_Init() {
  // CONFIGURATION:
    // ANALOG:
  analogReadResolution(12);
    // PWM:
  ledcSetup(0, 5000, 10);
  
  // GPIO:
    // INPUT:
  pinMode(Esp_Input, INPUT);
    // OUTPUT:
  ledcAttachPin(Esp_OutputPWM_16, 0);

  
}