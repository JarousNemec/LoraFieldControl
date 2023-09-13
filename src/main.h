#include <Arduino.h>
#include <SoftwareSerial.h>
#include "../lib/EBYTE/EBYTE.h"
#include "../lib/ArduinoJson/ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "../lib/ESP_EEPROM/src/ESP_EEPROM.h"
#include "SystemStructures.h"
#include <ESP8266HTTPClient.h>
#include "website.h"

#ifndef LORARANGEMETER_MAIN_H
#define LORARANGEMETER_MAIN_H

#define PIN_RX 14   //D5 on the board (Connect this to the EBYTE TX pin)
#define PIN_TX 12   //D6 on the board (connect this to the EBYTE RX pin)

#define PIN_M0 5    //D1 on the board (connect this to the EBYTE M0 pin)
#define PIN_M1 4    //D2 on the board (connect this to the EBYTE M1 pin)
#define PIN_AX 16   //D0 on the board (connect this to the EBYTE AUX pin)
#define PIN_SUCCESS_LED 13
#define PIN_SEND_LED 15
#define CONFIGURATION_ADDRESS 1
#define DISCOVER_TIMEOUT 12000
#define WATER_LEVEL_COLLECT_TIME 10000
#define PING_TIMEOUT 4000
#define OTHER_DATA_TIMEOUT 5000
#define OTHER_DATA_MAX_ATTEMPTS 3
#define LED_TIMEOUT 500
// you will need to define the pins to create the serial port
SoftwareSerial ESerial(PIN_RX, PIN_TX);

// create the transceiver object, passing in the serial and pins
EBYTE Transceiver(&ESerial, PIN_M0, PIN_M1, PIN_AX);

DynamicJsonBuffer jBuffer;
WiFiClient client;
HTTPClient http;

ConfigStruct  StationConfig;
PingStatusStruct PingStatus;
BeaconDiscoverStatusStruct BeaconDiscoverStatus;
OtherDataStatusStruct OtherDataStatus;
WaterLevelStatusStruct WaterLevelStatus;
String BeaconId = "";

const String ssid = "LoraNet";  // Enter SSID here
const String password = "12345678";  //Enter Password here

unsigned long shine_success_time = 0;
unsigned long shine_send_time = 0;

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

//String AssembleHTMLPage();

void HandleNotFound();

void HandleOnConnect();

StationType GetStationTypeFromString(const String &text);

String GetStationTypeFromEnum();

void LoadConfiguration();

void PrintConfig();

void SaveConfiguration();

bool ValidatePacket(JsonObject *pckt);

void ProcessPacket(JsonObject *pckt);

void ProcessPing(JsonObject *pckt);

void BehaveByStationType();

void BehaveAsPinger();

void SendPacket(PacketType type, const String &content, const String &source, const String &destination);

void ProcessDiscoverBeacon(JsonObject *pckt);

void ValidateTimeouts();

void ShineControl();

void ShineSend();

void ShineSuccess();

void BehaveAsFieldStation();

void ProcessOtherData(JsonObject *pckt);

void CollectSensorData();

FieldStationType GetFieldStationTypeFromString(const String &text);

String GetFieldStationTypeFromEnum();

bool IsWifiAvailable(const String &wifi_ssid);

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0

void BehaveAsBeacon();

void CollectWaterLevelSensorData();

#endif //LORARANGEMETER_MAIN_H


/*<<<<Ping Packet Struct>>>>
 * t - type
 * c - content
 * s - from device name
 * d - to device name
 * */
