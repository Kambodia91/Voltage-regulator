#include "managerWifi.h"
#include "defines.h"

#include <ArduinoLogger.h>                         // [Serial / Terminal]
#include <ArduinoJson.h>                           // https://github.com/bblanchon/ArduinoJson
#include <WiFiManager.h>                           // [Wifi Manager]
#include <FS.h>                                    // [Panięć FLASH] 
#include <SPIFFS.h>
#include <EEPROM.h>                                // [Panięć EEPROM] 
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WebServer.h>
String esp_name = "CONFIG-" + String(Name_ESP8266);

bool SetStaticIP = false;                          // Ustaw Statyczny Adres IP
bool shouldSaveConfig = false;
//------------------procedures---------------------//

// callback notifying us of the need to save config
void saveConfigCallback() {
  inf << "Should save config" << endl;
  shouldSaveConfig = true;
}
/*
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
*/
void spiffsFormat(){
  // clean Spiffs, for testing
  inf << "[SPIFFS] Start formating" << endl;
  if (SPIFFS.format()) {
  inf << "[SPIFFS] Formated" << endl;
  } else {
    err << "[SPIFFS] Formating error" << endl;
  }
}

void spiffsSetup() {
  //Serial.println("mounting FS...");

  if (SPIFFS.begin()) { 
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);


        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {

          Serial.println("\nparsed json");
          strcpy((char *)blynk_token, json["blynk_token"]);                                // Odczyt blynk_token z FS.
          
#ifdef BRIDGE_TOKEN
          strcpy((char *)bridge_token, json["bridge_token"]);                              // Odczyt bridge_token z FS.
#endif
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
      }
    } else {
      Serial.println("failed to mount FS");
    }
}


void managerReset() {
  // reset settings - for testing
   inf << "[WiFi] Reseting settings" << endl;
   WiFiManager wifiManager;
   wifiManager.resetSettings();
   inf << "[WiFi] Settings reseted" << endl;
}

void managerSetup() {
  inf << "[WiFi] Starting manager" << endl;
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);         // Nazwa Obiektu, Treść w oknie Obiektu "blynk token", Zawartość Wyświetlana Zdefiniowana w global.h, Długość Zawartośći.
#ifdef BRIDGE_TOKEN
  WiFiManagerParameter custom_bridge_token("bridge", "bridge token", bridge_token, 32);     // Nazwa Obiektu, Treść w oknie Obiektu "bridge token", Zawartość Wyświetlana Zdefiniowana w global.h, Długość Zawartośći.
#endif
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_blynk_token);                                            // Miejsce do wpisywania Blynk_Tokenu w MAnagerWifi.
#ifdef BRIDGE_TOKEN
  wifiManager.addParameter(&custom_bridge_token);                                           // Miejsce do wpisywania Bridge_Tokenu w MAnagerWifi.
#endif
  
  
  
  // IPAddress _ip, _gw, _sn;
  // _ip.fromString(static_ip);
  // _gw.fromString(static_gw);
  // _sn.fromString(static_sn);
  // // dns.fromString(static_gw);

  // if (SetStaticIP) { wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn); }

  // reset settings - for testing
  // wifiManager.resetSettings();
  inf << "[WiFi] waiting for clients" << endl;
  if (!wifiManager.autoConnect((char *)esp_name.c_str())) {
    err << "[WiFi] failed to connect and hit timeout" << endl;
//    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    //ESP.restart();
//    delay(5000);
  }

  // if you get here you have connected to the WiFi
  inf << "[WiFi] connected...yeey :)" << endl;

  // read updated parameters
  
  //strcpy((char *)blynk_token, custom_blynk_token.getValue());                               // Zapis z ManagerWifi do zmiennej blynk_token.
  
#ifdef BRIDGE_TOKEN
  //strcpy((char *)bridge_token, custom_bridge_token.getValue());                             // Zapis z ManagerWifi do zmiennej bridge_token.
#endif
  
  // save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument json(1024);
    json["blynk_token"] = blynk_token;                                                       // Zapis blynk_token Do FS
#ifdef BRIDGE_TOKEN
    json["bridge_token"] = bridge_token;                                                     // Zapis bridge_token Do FS
#endif

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    //end save
  }
}