#include <Arduino.h>

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
//----------------------------ESP Setings---------------------------//
#define Esp_Input                       1                                   // Input B.
#define Esp_OutputPWM_16                16                                  // PWM White Warm.
                

#define Esp_PWM_Max                     1023
#define Esp_PWM_Freq                    1000                                // 1Khz.
#define Esp_PWM_Res                     10                                  // 10bit = 1023

//-----------------------Regulator Setting--------------------------//
#define ALT_FILT_COEF                   655                         // battery voltage filter coefficient in fixed-point. coef_fixedPoint = coef_floatingPoint * 2^16. In this case 655 = 0.01 * 2^16
#define BAT_CELLS                       10                          // battery number of cells. Normal Hoverboard battery: 10s  
#define ALT_CALIB_ADC                   1495                        // input voltage measured by multimeter (multiplied by 100). In this case 43.00 V * 100 = 4300
#define ALT_CALIB_REAL_VOLTAGE          3947                        // adc-value measured by mainboard (value nr 5 on UART debug output)

#define PID_KP                          10                          // 
#define PID_KI                          2                           // 
#define PID_KD                          5                           // 
#define PID_LIMIT_MIN                   0                           // 
#define PID_LIMIT_MAX                   1023                        // 


//-------------------------Blynk Setings----------------------------//
#define BLYNK_PRINT Serial                                          // Defines the object that is used for printing
#define blynk_token             "o6uW3mPLvn_kHlFuyCeDox48s4sCUvxI"  // Token do ESP.
//#define BRIDGE_TOKEN                                                // Włączenie Mostu między Blynkiem.
#define bridge_token            "ieDu65nyAK1b2zeVFdpWcV18I7QeIBqy"  // Token od ESP.
#define blynk_bridge_pin        201                                 // Virtual Pin Bridge Blynk.
#define blynk_server            "kambodia.ddns.net"                 // Zewnętrzny Adres Servera Blynk.
//#define blynk_server            "192.168.1.10"                      // Lokalny Adres Servera Blynk.
#define blynk_port              "8080"                              // Port Servera Blynk
#define blynk_time_out          10                                  // Czas gdzy urzadzenie nie ma połączenia z serwerem Blynk nastepuje restat. (Czas w sekundach)


//--------------------------HTTP Update-----------------------------//
#define Http_ota_server         "kambodia.ddns.net"                 // Adres Http Servera OTA z Plikami *.bin
#define Http_ota_port           "80"                                // Port Http Servera OTA

//--------------------------Wifi Setings----------------------------//
#define MySsid                  "NETIA"                             // Nazwa Sieci
#define MyPass                  "Nikuda2518"                        // Hasło Sieci
#define terminal_name_device    "[ESP32] "                          // Nazwa Wyświetlana w Terminalu.
#define Name_ESP8266            "ESP32-Regulator"                           // Nazwa Hosta w Routerze.

//-----------------------WifiManager Setings------------------------//
#define BRIDGE_TOKEN                                              // Dodanie Do WifiManagera Mozliwość Wpisania Bridge Token.
#define static_ip               "192.168.1.50"                      // Ip Urządzenia
#define static_gw               "192.168.1.1"                       // Ip Routera
#define static_sn               "255.255.255.0"                     // Maska Podsieci

//---------------------------Esp Setings----------------------------//
// OneWire - Mam zjebane esp, normalnie powinno byc: SDA 4, SCL 5.
#define SpeedRate               115200                              // Prędkość Serial.
//ADC_MODE(ADC_VCC);                                                  // Pomiar Napięcia ADC.

//----------------------------End File------------------------------//