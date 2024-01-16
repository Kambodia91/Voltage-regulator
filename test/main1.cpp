//-----------------------------includes-------------------------//
#include <Arduino.h>                                            // [Arduino]
#include <ArduinoLogger.h>                                      // [Serial / Terminal]

#include "blynk.h"                                              // [Blynk]
#include "defines.h"                                            // [Define]
#include "managerWifi.h"                                        // [Manager WiFi]
#include "ota.h"                                                // [Ota WiFi]
#include "setup.h"                                              // [Setup]
#include "user.h"                                               // [User]

//--------------------------------------------------------------//

void setup() {
  UART_Init();                                                  // Uart
  LOGGER_Init();                                                // Logger
  GPIO_Init();                                                  // GPIO
  userpreSetup();                                               // Pre Setup
  // spiffsSetup();                                                // Memory
  // managerSetup();                                               // Manager WiFi
  // managerReset();                                               // Manager WiFi Reset Setting For Test
  blynkSetup();                                                 // Blynk
  otaSetup();                                                   // Ota FiFi
  timmerSetup();                                                // Blynk Timer
  userpostSetup();                                              // Post Setup
  logoBlynk();                                                  // Blynk Logo
}

//--------------------------------------------------------------//

void loop() {
  arduinoOtaHandle();                                           // Ota WiFi
  blynkRun();                                                   // Blynk Run
  userLoop();                                                   // User Loop
}
           
//--------------------------END MAIN----------------------------//
