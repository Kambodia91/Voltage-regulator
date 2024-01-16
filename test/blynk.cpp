#include "blynk.h"
#include "managerWifi.h"
#include <ArduinoLogger.h>                         // [Serial / Terminal]

#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <TimeLib.h>  
#include <BlynkSimpleEsp32.h>                      // [Blynk]
#include "setup.h"
#include "user.h"

WidgetBridge BlynkBridge (blynk_bridge_pin);      // [Most]
WidgetTerminal Terminal(V0);
// HTTPClient httpRead;        
// WiFiClient espClient;                                            // WebHook.
// HTTPClient httpWrite;                                                    // WebHook.
// HTTPClient httpWebHook;                            // WebHook.
BlynkTimer timer;                                                   // Nazwa Timera Blynk.
//WidgetRTC rtc;                                                      // Czas z Serwera Blynk.
// WiFiClient UpdateEspClient;

extern int adcAltValue; 
extern int setPoint;

// Zmienna z user.cpp
int            outputCompiute;




char ServerClock[20] = "";                                          //

int RealYear;
int RealMonth;
int RealDay;
int RealHour;
int RealMinute;
int RealSecond;
String DayOfWeak;
int DayOfWeakNumber;

bool TimeActivation = false;
unsigned long DifferenceTimer_1;
unsigned long DifferenceTimer_1_last = 0;
unsigned long Timer_1;
bool checkSpeedButton = false;                                      //
uint32_t SpeedTest;                                                 //
uint8_t Signal;                                                     // 
uint8_t LastSignal;                                                 //

void blynkSetup() {
  Blynk.begin(blynk_token, "NETIA", "nikuda2518",blynk_server, atoi(blynk_port));
  //Blynk.config(blynk_token, blynk_server, atoi(blynk_port));
  bool result = Blynk.connect();
  inf << "[BLYNK] Attempting connection to server at: " << blynk_server << endl;
    if (result != true) {
      err << "[BLYNK] Connection Fail, token: " << blynk_token << endl;
      // wifiManager.resetSettings();
      // ESP.reset();
      // delay (5000);
    } else {
      inf << "[BLYNK] Connection" << endl;
    }
}

void blynkRun() {
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Blynk.run();
  }
    timer.run();                                                      // Uruchomienie Czasówki z Blynk.
  // BlynkTimeOutRestart();

}

void logoBlynk() {
  inf << "    ___  __          __" << endl;
  inf << "   / _ )/ /_ _____  / /__" << endl;
  inf << "  / _  / / // / _ \\/  '_/" << endl;
  inf << " /____/_/\\_, /_//_/_/\\_\\" << endl;
  inf << "        /___/ ver. " << BLYNK_VERSION <<  endl;
  inf << " on " << BLYNK_INFO_DEVICE << " Instaled." << endl;
  inf << np << endl;
  Terminal.flush();
}
void BlynkTimeOutRestart() {
  unsigned long aktualnyCzas = millis();
  if (!Blynk.connected() == 1) {
    //necessaryShutdown();
    //Blynk.connect();
    if (TimeActivation == 0) {
        Timer_1 = aktualnyCzas;
        DifferenceTimer_1 = 0;
        DifferenceTimer_1_last =0;
        warn << np << "[BLYNK LOOP] Brak połączenia, prubuje wznowić połączenie." << endl;
        TimeActivation = 1; 
    }
    DifferenceTimer_1 = (aktualnyCzas - Timer_1)/1000;
    if (DifferenceTimer_1 != DifferenceTimer_1_last) {
      warn << np << "[BLYNK LOOP] Odliczanie: " << DifferenceTimer_1 << "/" << blynk_time_out << endl;
      DifferenceTimer_1_last = DifferenceTimer_1;
    } 
    if (DifferenceTimer_1 >= blynk_time_out) { // Po odliczeniu jest restart ESP
      //warn << np << "[BLYNK LOOP] Connection Fail, token: " << blynk_token << endl;
      //warn << np << "[BLYNK LOOP] Restart." << endl;
      warn << "[BLYNK LOOP] Reconnecting WiFi." << endl;
      DifferenceTimer_1 = 0;
      
      //delay(1000);
      ESP.restart();
    } else {
        // 
    }
  } else {
    if (TimeActivation == 1) {
      String body = String("[BLYNK LOOP] ") + Name_ESP8266 + String(" - Połączenie przywrócone.");
      inf << np << body << endl;
      // Blynk.notify(body);
    }
    DifferenceTimer_1 = 0;
    TimeActivation = 0;

  }
}

void RealTimeClock() {                                              // Odczty Czasu Z Servera.
  RealYear = year();                                                // Odczty Rok.
  RealMonth = month();                                              // Odczty Miesiąc.
  RealDay = day();                                                  // Odczty Dzień.
  RealHour = hour();                                                // Odczty Godzina.
  RealMinute = minute();                                            // Odczty Minuta.
  RealSecond = second();                                            // Odczty Sekunda.
  DayOfWeakNumber = weekday();                                      // Odczty Dzień Tygodnia.
  
  sprintf(ServerClock,"%04d.%02d.%02d %02d:%02d:%02d",RealYear, RealMonth, RealDay, RealHour, RealMinute, RealSecond);


  switch (DayOfWeakNumber)                                          // Zmiana Numeru Dnia Tygodnia Na Nazwy Tygodnia.
  {
  case 1:
    DayOfWeak = String("Niedziela");
    break;
  case 2:
    DayOfWeak = String("Poniedziałek");
    break;
  case 3:
    DayOfWeak = String("Wtorek");
    break;
  case 4:
    DayOfWeak = String("Środa");
    break;
  case 5:
    DayOfWeak = String("Czwartek");
    break;
  case 6:
    DayOfWeak = String("Piątek");
    break;
  case 7:
    DayOfWeak = String("Sobota");
    break;
  
  default:
    break;
  }
}

void timmerSetup() {
  timer.setInterval(1000L, CheckCycleESP);
  timer.setInterval(1000L, WifiSignal);
  timer.setInterval(500L, widgetBlynk);
}

void widgetBlynk() {
  Blynk.virtualWrite(V3, outputCompiute);   
}

void WifiSignal() {
  int quality;
  int MyRSSI;
  int LastRSSI; 

  MyRSSI = WiFi.RSSI();
  if (MyRSSI <= -100) {
    quality = 0;
  } else if (MyRSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (MyRSSI + 100);
  }
  if (MyRSSI == LastRSSI) { 
    //
  } else {
    Blynk.virtualWrite(V101, quality); 
    LastRSSI = MyRSSI;
  }
   
}

void CheckCycleESP() {
  if (checkSpeedButton == true) {
    inf << np << terminal_name_device << "Cykli na 1s: " << SpeedTest << endl;
    SpeedTest = 0;
  }
}
BLYNK_WRITE(V0) {
  if (String("help") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "For more information on a specific command:" << endl;
    inf << np << terminal_name_device << "clr - Wyczyść Terminal." << endl;
    inf << np << terminal_name_device << "update - Update Ota Software z Serwera." << endl;
    inf << np << terminal_name_device << "esp - Informacje o ESP." << endl;
    inf << np << terminal_name_device << "wifi - Informacje o WiFi." << endl;
    inf << np << terminal_name_device << "blynk - Informacje o Blynk." << endl;
    inf << np << terminal_name_device << "ota - Informacje o Ota." << endl;
    inf << np << terminal_name_device << "virtual - Informacje o Virtual Pin." << endl;
    inf << np << terminal_name_device << "analog - Informacje o Analogowych Pinach." << endl;
    inf << np << terminal_name_device << "digital - Informacje o Cyfrowych Pinach." << endl;
    inf << np << terminal_name_device << "format - Format danych w pamieci flash." << endl;
    inf << np << terminal_name_device << "reset - Kasowanie ustawień Wifi Managera." << endl;
    inf << np << terminal_name_device << "log err - Pierwszy poziom powiadomień." << endl;
    inf << np << terminal_name_device << "log warn - Drugi poziom powiadomień." << endl;
    inf << np << terminal_name_device << "Log inf - Trzeci poziom powiadomień." << endl;
    inf << np << terminal_name_device << "log trac - Czwarty poziom powiadomień." << endl;
    inf << np << terminal_name_device << "log verb - Piąty poziom powiadomień." << endl;

  } else if (String("clr") == param.asStr()) {
    Terminal.clear();
  } else if (String("update") == param.asStr()) {
    inf << "Start updating" << endl;
    //necessaryShutdown();
    delay(1000);
//    HTTPUpdate();
  } else if (String("restart") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "Restarting..." << endl;
    //necessaryShutdown();
    delay(1000);
    ESP.restart();
  } else if (String("date") == param.asStr()) {
    RealTimeClock();
    inf << endl;
    inf << np << terminal_name_device << "Actual Year: " << RealYear << endl;
    inf << np << terminal_name_device << "Actual Month: " << RealMonth << endl;
    inf << np << terminal_name_device << "Actual Day: " << RealDay << endl;
    inf << np << terminal_name_device << "Actual Hour: " << RealHour << endl;
    inf << np << terminal_name_device << "Actual Minute: " << RealMinute << endl;
    inf << np << terminal_name_device << "Actual Second: " << RealSecond << endl;
    inf << np << terminal_name_device << "Actual Day Of Weak Number: " << DayOfWeakNumber << endl;
    inf << np << terminal_name_device << "Actual Day Of Weak: " << DayOfWeak << endl;
  } else if (String("esp") == param.asStr()) {
    inf << endl;
    // inf << np << terminal_name_device << "ChipID: " << ESP.getChipId() << endl;
    // inf << np << terminal_name_device << "Core ver: " << ESP.getCoreVersion() << endl;
    // inf << np << terminal_name_device << "Chip size: " << ESP.getFlashChipRealSize() << endl;
    // inf << np << terminl_name_device << "Reset info: " << ESP.getResetInfo() << endl;
    // inf << np << terminal_name_device << "Reset reason: " << ESP.getResetReason() << endl;
    // inf << np << terminal_name_device << "Vcc: " << (ESP.getVcc()/1024.00f) << "V" << endl;
    inf << np << terminal_name_device << "SDK ver: " << ESP.getSdkVersion() << endl;
    inf << np << terminal_name_device << "Speed Rate Terminal USB: " << SpeedRate << endl;
  } else if (String("wifi") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "IP Address: " << WiFi.localIP().toString() << endl;
    inf << np << terminal_name_device << "MAC Address: " << WiFi.macAddress() << endl;
    inf << np << terminal_name_device << "Name: " << WiFi.softAPSSID() << endl;
    inf << np << terminal_name_device << "SSID: " << WiFi.SSID() << endl;
    inf << np << terminal_name_device << "Signal: " << WiFi.RSSI() << endl;
    inf << np << terminal_name_device << "Status: " << WiFi.status() << endl;
    inf << np << terminal_name_device << "Gateway: " << WiFi.gatewayIP().toString() << endl;
    inf << np << terminal_name_device << "DNS: " << WiFi.dnsIP().toString() << endl;
    //inf << np << terminal_name_device << "Hostname: " << WiFi.hostname() << endl;
  } else if (String("blynk") == param.asStr()) {
    inf << endl;
    inf << np << "    ___  __          __" << endl;
    inf << np << "   / _ )/ /_ _____  / /__" << endl;
    inf << np << "  / _  / / // / _ \\/  '_/" << endl;
    inf << np << " /____/_/\\_, /_//_/_/\\_\\" << endl;
    inf << np << "        /___/" << endl;
    inf << np << endl;
    inf << np << terminal_name_device << "Blynk Version: " << BLYNK_VERSION << endl;
    inf << np << terminal_name_device << "Blynk Token Device: " << blynk_token << endl;
    inf << np << terminal_name_device << "Blynk Server Address: " << blynk_server << endl;
    inf << np << terminal_name_device << "Blynk Server Port: " << blynk_port << endl;
  } else if (String("ota") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "Ota Server Address: " << Http_ota_server << endl;
    inf << np << terminal_name_device << "Ota Server Port: " << Http_ota_port << endl;  
  }  else if (String("format") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "LittleFSFormat" << endl;
    spiffsFormat();
  } else if (String("reset") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "ManagerReset" << endl;
    managerReset();
  } else if (String("log err") == param.asStr()) {
    logger.edit(Terminal, LOG_LEVEL_ERROR);
  } else if (String("log warn") == param.asStr()) {
    logger.edit(Terminal,LOG_LEVEL_WARNING);
  } else if (String("log info") == param.asStr()) {
    logger.edit(Terminal, LOG_LEVEL_INFO);
  } else if (String("log trac") == param.asStr()) {
    logger.edit(Terminal, LOG_LEVEL_TRACE);
  } else if (String("log verb") == param.asStr()) {
    logger.edit(Terminal, LOG_LEVEL_VERBOSE);
  } else if (String("virtual") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "All Virtual Pins:" << endl;
    inf << np << terminal_name_device << "Blynk Pins:" << endl;
    inf << np << terminal_name_device << "V0: Terminal" << endl;
    inf << np << terminal_name_device << "V1: " << endl;
    inf << np << terminal_name_device << "V2: " << endl;
    inf << np << terminal_name_device << "V3: " << endl;
    inf << np << terminal_name_device << "V4: " << endl;
    inf << np << terminal_name_device << "V100: Guzik Sprawdź szybkość programu" << endl;
    // inf << np << terminal_name_device << "Blynk Bridge Pins:" << endl;
    // inf << np << terminal_name_device << "V3: Wysyłanie Stanu Wciśnięcia Button2 do BlynkBridge" << endl;
    // inf << np << terminal_name_device << "V4: Wysyłanie Stanu Wciśnięcia Button1 do BlynkBridge" << endl;
    inf << np << terminal_name_device << "V101: Wyświetlanie Zasięgu Wifi" << endl;
  } else if (String("analog") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "All Analog Pins:" << endl;
    inf << np << terminal_name_device << "ADC_0: Pomiar Vcc 3.3V" << endl;
  } else if (String("digital") == param.asStr()) {
    inf << endl;
    inf << np << terminal_name_device << "All Digital Pins:" << endl;
    inf << np << terminal_name_device << "GP_0: Flash Mode" << endl;
    inf << np << terminal_name_device << "GP_1: TX" << endl;
    inf << np << terminal_name_device << "GP_2: --" << endl;
    inf << np << terminal_name_device << "GP_3: RX" << endl;
    inf << np << terminal_name_device << "GP_4: Protocol I2C SDA" << endl;
    inf << np << terminal_name_device << "GP_5: Protocol I2C SCL" << endl;
    inf << np << terminal_name_device << "GP_6: Memory Flash." << endl;
    inf << np << terminal_name_device << "GP_7: Memory Flash." << endl;
    inf << np << terminal_name_device << "GP_8: Memory Flash." << endl;
    inf << np << terminal_name_device << "GP_9: Memory Flash." << endl;
    inf << np << terminal_name_device << "GP_10: Memory Flash." << endl;
    inf << np << terminal_name_device << "GP_11: Memory Flash." << endl;
    //inf << np << terminal_name_device << "GP_12: Output PWM Motor_3 Value = " << MecanumPWM_FR << endl;
    //inf << np << terminal_name_device << "GP_13: Output PWM Motor_2 Value = " << MecanumPWM_FL << endl;
    //inf << np << terminal_name_device << "GP_14: Output PWM Motor_1 Value = " << MecanumPWM_RR << endl;
    //inf << np << terminal_name_device << "GP_15: Output PWM Motor_4 Value = " << MecanumPWM_RL << endl;
    inf << np << terminal_name_device << "GP_16: WakeUp" << endl;
  } else if (String("scan") == param.asStr()) {
    inf << np << endl;
    //scaner();
  }else {
    inf << np << endl;
    inf << np << terminal_name_device << "Unkown command: " << param.asStr() << endl;
  }
}

//-----------------------------BLYNK--------------------------------//
BLYNK_CONNECTED() {
  //rtc.begin();                                                      // Synchronize time on connection.
  //Blynk.syncVirtual(V1, V2);                           // Synchronizowanie Wartości z Aplikacji Blynk.

 // BlynkBridge.setAuthToken(bridge_token);                           // Token do ESP-Rolety_L923D.
}

BLYNK_WRITE(V1) { adcAltValue = param.asInt(); }
BLYNK_WRITE(V2) { setPoint = param.asInt(); }
// BLYNK_WRITE(V3) { TimeRampDown = param.asInt(); }
// BLYNK_WRITE(V3) { TimeRampDown = param.asInt(); }

//BLYNK_WRITE(V21) { = param.asInt(); }                             // BlynkBridge Pin.
BLYNK_WRITE(V100) { SpeedTest = 0;                                  // Przycisk w Aplikacji Blynk.
                  checkSpeedButton = param.asInt(); }
//BLYNK_WRITE(V101) { = param.asInt(); }                            // Zasięg WiFi Wysyłany do Aplikacji.

//--------------------KONIEC PLIKU GLOBALSLIB-----------------------//
//-----------------end main file-------------------//