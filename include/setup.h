#ifndef SETUP_H
#define SETUP_H

#include <Arduino.h>
#include <ArduinoLogger.h>                         // [Serial / Terminal]

void SerialSetup();
void GpioInit();
void BlynkSetup();
void BlynkTimeOutRestart();
void BlynkLogo();
void BlynkLoop();
void BlynkTimerSet();
void BlynkTerminal(String cmd);
void BlynkTerminal(int cmd);
void SaveConfigCallback();
void flashCheck();
void FsFormat();
void FsSetup();
void ManagerReset();
void ManagerSetup();
void WidgetTest();
void WifiSignal();
void CheckCycleESP();
void HttpUpdateStart();
void OtaSetup();
void RealTimeClock();
void setupPlatform();
void loopPlatform();


void controlTracks(int difference);
#endif