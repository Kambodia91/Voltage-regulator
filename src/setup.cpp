//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------ 
#include "defines.h"
#include "config.h"
#include "setup.h"

#include "voltageRegulator.h"

#if defined(VARIANT_ESP32_S2)
#include <BlynkSimpleEsp32.h>                      // [Blynk]
#endif

#if defined(VARIANT_ESP32_LOLIN)
#include <ArduinoJson.h>                           // https://github.com/bblanchon/ArduinoJson
#include <LITTLEFS.h>                              // [Panięć FLASH] 
// #include <FS.h>                                    // [Panięć FLASH] 
// #include <SPIFFS.h>
#include <EEPROM.h>                                // [Panięć EEPROM] 
#include <ArduinoOTA.h>                            // [Ota Wifi Local]
#include <BlynkSimpleEsp32.h>                      // [Blynk]
#include <WidgetRTC.h>
#include <DNSServer.h>
#include <ESPmDNS.h>

#include <WebServer.h>
#include <WiFiManager.h>                           // [Wifi Manager]
#include <ArduinoLogger.h>                         // [Serial / Terminal]

#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>                           // [Ota HTTP]
#endif

#if defined(VARIANT_ESP8266)
#include <LittleFS.h>                              // [Panięć FLASH] 
#include <EEPROM.h>                                // [Panięć EEPROM] 
//#include <SPIFFS.h>
#include <Arduino.h>                               // [Arduino]
#include <ArduinoJson.h>                           // https://github.com/bblanchon/ArduinoJson

#include <ArduinoOTA.h>                            // [Ota Wifi Local]
#include <BlynkSimpleEsp8266.h>                    // [Blynk]
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>                           // [Wifi Manager]
#include <ArduinoLogger.h>                         // [Serial / Terminal]

#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>                     // [Ota Połączenie Do Serwera]
#include <ESP8266httpUpdate.h>                     // [Ota HTTP]
#endif

//------------------------------------------------------------------------
// objects
//------------------------------------------------------------------------ 
#if defined(VARIANT_ESP32_S2) || defined(VARIANT_ESP32_LOLIN) || defined(VARIANT_ESP8266)
WidgetTerminal Terminal(V0);
BlynkTimer timer;   // Nazwa Timera Blynk.
WidgetBridge BlynkBridge (blynk_bridge_pin);       // [Most]
//WidgetRTC rtc;
#endif


#if defined(VARIANT_ESP32_LOLIN) || defined(VARIANT_ESP8266)
WiFiClient espClient;
HTTPClient httpRead;                               // WebHook.
HTTPClient httpWrite;                              // WebHook.
HTTPClient http;                                   // WebHook.
WiFiClient UpdateEspClient;
HTTPClient httpWebHook;                            // WebHook.
#endif

#if defined(VARIANT_ESP8266)
ESP8266HTTPUpdate httpUpdate;
#endif

//------------------------------------------------------------------------
// external variables
//------------------------------------------------------------------------ 
extern int BLYNK_SET_POINT;
extern int BLYNK_PID_KP;
extern int BLYNK_PID_KI;
extern int BLYNK_PID_KD;

//------------------------------------------------------------------------
// variables
//------------------------------------------------------------------------ 
String esp_name = "CONFIG-" + String(Name_ESP);

bool SetStaticIP = false;                          // Ustaw Statyczny Adres IP
bool shouldSaveConfig = false;
bool TimeActivation = false;
unsigned long DifferenceTimer_1;
unsigned long DifferenceTimer_1_last = 0;
unsigned long Timer_1;

char ServerClock[20] = "";

uint8_t Signal;
uint8_t LastSignal;

bool checkSpeedButton = false;
uint32_t SpeedTest;

uint16_t RealYear;
uint8_t RealMonth;
uint8_t RealDay;
uint8_t RealHour;
uint8_t RealMinute;
uint8_t RealSecond;
String DayOfWeak;
uint8_t DayOfWeakNumber;

//------------------------------------------------------------------------
// procedures serial setup
//------------------------------------------------------------------------ 
void SerialSetup() {
  Serial.begin(SpeedRate);
  Serial.println("Setup Serial Begin.");
  delay(100);
  logger.add(Serial, LOG_LEVEL_INFO, true); // This will log everything on Serial
  logger.add(Terminal, LOG_LEVEL_VERBOSE, true); // This will log everything on Serial
  logger.disableLevelName(Terminal);
}

//------------------------------------------------------------------------
// procedures gpio initialize
//------------------------------------------------------------------------ 
void GpioInit() {
#if defined(VARIANT_ESP32_S2)
    analogReadResolution(ANALOG_IN_RES);
    
    ledcSetup(0, PWM_FREQ, PWM_RES);
    ledcAttachPin(OUTPUT_PIN, 0);
#endif

    // pinMode(SENSOR_PIN, INPUT);
    // pinMode(OUTPUT_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}

//------------------------------------------------------------------------
// procedures blynk setup
//------------------------------------------------------------------------ 
void BlynkSetup() {
    Blynk.begin(blynk_token, MySsid, MyPass, blynk_server, atoi(blynk_port));
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
//------------------------------------------------------------------------
// procedures blynk timeout restart
//------------------------------------------------------------------------ 
void BlynkTimeOutRestart() {
  unsigned long aktualnyCzas = millis();
  if (!Blynk.connected() == 1) {
    Blynk.connect();
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
      warn << np << "[BLYNK LOOP] Connection Fail, token: " << blynk_token << endl;
      warn << np << "[BLYNK LOOP] Restart." << endl;
      delay(200);
      ESP.restart();
    } else {
        // 
    }
  } else {
    if (TimeActivation == 1) {
      String body = String("[BLYNK LOOP] ") + Name_ESP + String(" - Połączenie przywrócone.");
      inf << np << body << endl;
      // Blynk.notify(body);
    }
    DifferenceTimer_1 = 0;
    TimeActivation = 0;
  }
}

//------------------------------------------------------------------------
// procedures blynk logo
//------------------------------------------------------------------------ 
void BlynkLogo() {
    inf << "    ___  __          __" << endl;
    inf << "   / _ )/ /_ _____  / /__" << endl;
    inf << "  / _  / / // / _ \\/  '_/" << endl;
    inf << " /____/_/\\_, /_//_/_/\\_\\" << endl;
    inf << "        /___/ ver. " << BLYNK_VERSION <<  endl;
    inf << " on " << BLYNK_INFO_DEVICE << " Instaled." << endl;
    inf << np << endl;
}

//------------------------------------------------------------------------
// procedures blynk timer set
//------------------------------------------------------------------------ 
void BlynkTimerSet() {
    //timer.setInterval(20L, VoltageRegulatorCompiute);
    timer.setInterval(1000L, CheckCycleESP);
    timer.setInterval(1000L, WifiSignal);
    timer.setInterval(1000L, WidgetTest);
}

//------------------------------------------------------------------------
// procedures blynk loop
//------------------------------------------------------------------------ 
void BlynkLoop() {
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        Blynk.run();
    }
    timer.run();
}

//------------------------------------------------------------------------
// procedures blynk widget show
//------------------------------------------------------------------------ 
void WidgetTest(){

  // Blynk.virtualWrite(V61, Feedback_Serial1.batVoltage);  
  // Blynk.virtualWrite(V62, Feedback_Serial1.batVoltage);  
  // Blynk.virtualWrite(V63, Feedback_Serial1.boardTempSlave);  

  // Blynk.virtualWrite(V71, Feedback_Serial2.batVoltage);  
  // Blynk.virtualWrite(V72, Feedback_Serial2.boardTempMaster);  
  // Blynk.virtualWrite(V73, Feedback_Serial2.boardTempSlave); 

  // Blynk.virtualWrite(V32, OutputLewa);
  // Blynk.virtualWrite(V33, OutputPrawa);

}

//------------------------------------------------------------------------
// procedures blynk terminal command
//------------------------------------------------------------------------ 
void BlynkTerminal(String cmd) {
    String Cmd = cmd;
    Blynk.virtualWrite(V0, Cmd);
}

void BlynkTerminal(int cmd) {
    int Cmd = cmd;
    Blynk.virtualWrite(V0, Cmd);
}

//------------------------------------------------------------------------
// procedures blynk declaration virtual pin
//------------------------------------------------------------------------ 
BLYNK_CONNECTED() {
  //rtc.begin();                                                      // Synchronize time on connection.
  Blynk.syncVirtual(V1, V2, V3, V4);                           // Synchronizowanie Wartości z Aplikacji Blynk.

  //BlynkBridge.setAuthToken(bridge_token);                           // Token do ESP-Rolety_L923D.
}

BLYNK_WRITE(V1) { BLYNK_SET_POINT = param.asInt(); }
BLYNK_WRITE(V2) { BLYNK_PID_KP = param.asInt(); }
BLYNK_WRITE(V3) { BLYNK_PID_KI = param.asInt(); }
BLYNK_WRITE(V4) { BLYNK_PID_KD = param.asInt(); }

//BLYNK_WRITE(V21) { = param.asInt(); }                             // BlynkBridge Pin.
//BLYNK_WRITE(V100) { SpeedTest = 0;                                  // Przycisk w Aplikacji Blynk.
//                 checkSpeedButton = param.asInt(); }
//BLYNK_WRITE(V101) { = param.asInt(); }                            // Zasięg WiFi Wysyłany do Aplikacji.

//------------------------------------------------------------------------
// procedures wifi signal check
//------------------------------------------------------------------------ 
void WifiSignal() {
  Signal = (100 - abs(WiFi.RSSI()));
  if (Signal != LastSignal) {
    Blynk.virtualWrite(V101, Signal);
    LastSignal = Signal;
  } 
}

//------------------------------------------------------------------------
// procedures cycle esp check
//------------------------------------------------------------------------ 
void CheckCycleESP() {
  if (checkSpeedButton == true) {
    inf << np << terminal_name_device << "Cykli na 1s: " << SpeedTest << endl;
    SpeedTest = 0;
  }
}

//------------------------------------------------------------------------
// procedures save config wifimanager
//------------------------------------------------------------------------ 
void SaveConfigCallback() {
  inf << "Should save config" << endl;
  shouldSaveConfig = true;
}

//------------------------------------------------------------------------
// procedures flash check only esp8266
//------------------------------------------------------------------------ 
#if defined(VARIANT_ESP8266)
void flashCheck() {
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();

  inf << "[FLASH] Cheking.." << endl;
  inf << np << "real_id = " << ESP.getFlashChipId() << endl;
  inf << np << "real_size = " << realSize << " bytes" << endl;

  inf << np << "ide_size = " << ideSize << " bytes" << endl;
  inf << np << "ide_speed = " << ESP.getFlashChipSpeed() << " Hz" << endl;
  inf << np << "ide_mode = " <<
                (ideMode == FM_QIO
                     ? "QIO"
                     : ideMode == FM_QOUT
                           ? "QOUT"
                           : ideMode == FM_DIO
                                 ? "DIO"
                                 : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN") << endl;

  if (ideSize != realSize) {
    err << "[FLASH] Chip configuration wrong!" << endl;
  } else {
    inf << "[FLASH] Chip configuration ok." << endl;
  }
}
#endif

//------------------------------------------------------------------------
// procedures flash format
//------------------------------------------------------------------------ 
void FsFormat(){
#if defined(VARIANT_ESP32_LOLIN) || defined(VARIANT_ESP8266)
  // clean FS, for testing
  inf << "[LITTLEFS] Start formating" << endl;
  if (LITTLEFS.format()) {
  inf << "[LITTLEFS] Formated" << endl;
  } else {
    err << "[LITTLEFS] Formating error" << endl;
  }
#endif
}

//------------------------------------------------------------------------
// procedures flash setup
//------------------------------------------------------------------------ 
void FsSetup() {
#if defined(VARIANT_ESP32_LOLIN) || defined(VARIANT_ESP8266)
  // read configuration from FS json
  inf << "[LITTLEFS] mounting FS..." << endl;

  if (LITTLEFS.begin()) {
    inf << "[LITTLEFS] mounted file system" << endl;
    if (LITTLEFS.exists("/config.json")) {
      // file exists, reading and loading
      inf << "[LITTLEFS] reading config file" << endl;
      File configFile = LITTLEFS.open("/config.json", "r");
      if (configFile) {
        inf << "[LITTLEFS] opened config file" << endl;
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, buf.get());
        trace << buf.get() << endl;
        //serializeJsonPretty(doc, Serial);
        if (!doc.isNull()) {
          inf << "[LITTLEFS] parsed json" << endl;

          strcpy((char *)blynk_token, doc["blynk_token"]);                                // Odczyt blynk_token z FS.
          
          #ifdef BRIDGE_TOKEN
          strcpy((char *)bridge_token, doc["bridge_token"]);                              // Odczyt bridge_token z FS.
          #endif

        } else {
          err << "[LITTLEFS] failed to load json config" << endl;
        }
        configFile.close();
      }
    }
  } else {
    err << "[LITTLEFS] failed to mount FS" << endl;
  }
#endif
}

//------------------------------------------------------------------------
// procedures wifi manager reset
//------------------------------------------------------------------------ 
#if !defined(VARIANT_ESP32_S2)
void ManagerReset() {
  // reset settings - for testing
   inf << "[WiFi] Reseting settings" << endl;
   WiFiManager wifiManager;
   wifiManager.resetSettings();
   inf << "[WiFi] Settings reseted" << endl;
}

//------------------------------------------------------------------------
// procedures wifi manager setup
//------------------------------------------------------------------------ 
void ManagerSetup() {
  inf << "[WiFi] Starting manager" << endl;
#if defined(VARIANT_ESP8266)
    wifi_station_set_hostname(Name_ESP);                            // Nazwa Hosta Wyświetlana w Routerze.
#endif
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);         // Nazwa Obiektu, Treść w oknie Obiektu "blynk token", Zawartość Wyświetlana Zdefiniowana w global.h, Długość Zawartośći.
  
  #ifdef BRIDGE_TOKEN
  WiFiManagerParameter custom_bridge_token("bridge", "bridge token", bridge_token, 32);     // Nazwa Obiektu, Treść w oknie Obiektu "bridge token", Zawartość Wyświetlana Zdefiniowana w global.h, Długość Zawartośći.
  #endif
  
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(SaveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);                                            // Miejsce do wpisywania Blynk_Tokenu w MAnagerWifi.
  
  #ifdef BRIDGE_TOKEN
  wifiManager.addParameter(&custom_bridge_token);                                           // Miejsce do wpisywania Bridge_Tokenu w MAnagerWifi.
  #endif
  
  wifiManager.setDebugOutput(false);
  

  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  // dns.fromString(static_gw);

  if (SetStaticIP) {
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  }

  // reset settings - for testing
  // wifiManager.resetSettings();
  inf << "[WiFi] waiting for clients" << endl;
  if (!wifiManager.autoConnect((char *)esp_name.c_str())) {
    err << "[WiFi] failed to connect and hit timeout" << endl;
//    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
//    delay(5000);
  }

  // if you get here you have connected to the WiFi
  inf << "[WiFi] connected...yeey :)" << endl;

  // read updated parameters
  
  strcpy((char *)blynk_token, custom_blynk_token.getValue());                               // Zapis z ManagerWifi do zmiennej blynk_token.
  
  #ifdef BRIDGE_TOKEN
  strcpy((char *)bridge_token, custom_bridge_token.getValue());                             // Zapis z ManagerWifi do zmiennej bridge_token.
  #endif
  
  // save the custom parameters to FS
  if (shouldSaveConfig) {
    inf << "saving config" << endl;
    DynamicJsonDocument doc(1024);
    
    doc["blynk_token"] = blynk_token;                                                       // Zapis blynk_token Do FS
    
    #ifdef BRIDGE_TOKEN
    doc["bridge_token"] = bridge_token;                                                     // Zapis bridge_token Do FS
    #endif

    File configFile = LITTLEFS.open("/config.json", "w");

    if (!configFile) {
      warn << "failed to open config file for writing" << endl;
    }

    //serializeJsonPretty(doc, Terminal);
    serializeJson(doc, configFile);
    configFile.close();
    // end save
  }
}

//------------------------------------------------------------------------
// procedures http update from server
//------------------------------------------------------------------------ 
void HttpUpdateStart() {
  if (WiFi.status() != WL_CONNECTED) return;          
    t_httpUpdate_return ret = httpUpdate.update(UpdateEspClient, Http_ota_server, atoi(Http_ota_port), "/UpdateOTA/" blynk_token "/firmware.bin");
      switch(ret) {
        case HTTP_UPDATE_FAILED:
           err << "HTTP_UPDATE_FAILED Error (" << httpUpdate.getLastError() << "):" << httpUpdate.getLastErrorString().c_str() << endl;
           break;
       case HTTP_UPDATE_NO_UPDATES:
           inf << "HTTP_UPDATE_NO_UPDATES" << endl;
           break;
       case HTTP_UPDATE_OK:
           inf << "HTTP_UPDATE_OK" << endl;
           break;
    }
}

//------------------------------------------------------------------------
// procedures ota setup
//------------------------------------------------------------------------ 
void OtaSetup() {
#if defined(VARIANT_ESP8266)
  ArduinoOTA.setHostname((char *)esp_name.c_str());
#endif

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    err << "[OTA] Connection Failed! Rebooting..." << endl;
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.onStart([]() { inf << "[OTA] Start" << endl; });
  ArduinoOTA.onEnd([]() { inf << "[OTA] End" << endl; });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    inf << "[OTA] Progress:" << (progress / (total / 100)) << endl;
  });
  ArduinoOTA.onError([](ota_error_t error) {
    err << "[OTA] Error[" << error << "]:";
    if (error == OTA_AUTH_ERROR)
      err << np << "Auth Failed" << endl;
    else if (error == OTA_BEGIN_ERROR)
      err << np << "Begin Failed" << endl;
    else if (error == OTA_CONNECT_ERROR)
      err << np << "Connect Failed" << endl;
    else if (error == OTA_RECEIVE_ERROR)
      err << np << "Receive Failed" << endl;
    else if (error == OTA_END_ERROR)
      err << np << "End Failed" << endl;
  });
  ArduinoOTA.begin();
  inf << "[OTA] Ready for OTA update" << endl;
  inf << np << "[OTA] upload_protocol = espota" << endl;
  inf << np << "[OTA] upload_port = " << WiFi.localIP().toString() << endl;
}

//------------------------------------------------------------------------
// procedures rtc from server blynk
//------------------------------------------------------------------------ 
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
#endif

//------------------------------------------------------------------------
// procedures setup
//------------------------------------------------------------------------ 
void setupPlatform() {
    SerialSetup();
    GpioInit();
    BlynkSetup();
    BlynkLogo();
    BlynkTimerSet();
    #if defined(VARIANT_ESP8266) || defined(VARIANT_ESP32_LOLIN)
    //ManagerSetup();
    // FsSetup();
    //OtaSetup();
    #endif
}

//------------------------------------------------------------------------
// procedures loop
//------------------------------------------------------------------------ 
void loopPlatform() {
    BlynkLoop();
    SpeedTest = (SpeedTest+1);
    #if defined(VARIANT_ESP8266) || defined(VARIANT_ESP32_LOLIN)
    //ArduinoOTA.handle();
    #endif
    BlynkTimeOutRestart();
    Terminal.flush();
}
//------------------------------------------------------------------------
// end file
//------------------------------------------------------------------------ 