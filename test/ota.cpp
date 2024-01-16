#include "ota.h"
#include <ArduinoOTA.h>                            // [Ota Wifi Local]
#include <ArduinoLogger.h>                         // [Serial / Terminal]
#include <WiFi.h>


void arduinoOtaHandle() {
  ArduinoOTA.handle();
}

void otaSetup() {

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
     inf << "[OTA] Start" << endl;
    })
    .onEnd([]() {
      inf << "[OTA] End" << endl;
    })
    .onProgress([](unsigned int progress, unsigned int total) {
       inf << "[OTA] Progress:" << (progress / (total / 100)) << endl;
    })
    .onError([](ota_error_t error) {
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
  inf << "[OTA] upload_protocol = espota" << endl;
  inf << "[OTA] upload_port = " << WiFi.localIP().toString() << endl; 
}

