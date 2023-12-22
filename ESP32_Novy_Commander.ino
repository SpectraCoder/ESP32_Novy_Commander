/// The main goal of this script is to control the lights of the Novy cooking hood when the Zigbee kitchen lights are turned on or off.
/// This script looks at the API of Home Assistant to determine the on or off state of the kitchen lights, and sends a 433mhz signal to the connected 433mhz transmitter.
/// The ip address of the ESP32 shows a basic status web page with the ability to reboot the device.
/// SpectraCoder 01-12-2023

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <RCSwitch.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/// IMPORTANT! This file holds all credentials and settings.
/// Make a copy of config.example.h and rename it to config.h to make this work with your own setup.
#include "config.h"
#include "favicon.h"

bool isKitchenLightsOn = false;
int kitchenLightBrightness = 0;
bool isNovyLightOn = false;

WebServer server(80);

RCSwitch transmitter = RCSwitch();

String returnedAPIString = "";

void setup() {
  Serial.begin(115200);
  pinMode(POWER_433MHZ_PIN, OUTPUT);
  digitalWrite(POWER_433MHZ_PIN, HIGH);

  connectWifi();
  startServer();

  transmitter.enableTransmit(TRANSMIT_433MHZ_PIN);
  transmitter.setPulseLength(350);
  transmitter.setProtocol(12);

  ArduinoWifiUpdater();
}

///This function allows for firmware uploads through wifi,
///instead of connecting the ESP32 directly via usb.
void ArduinoWifiUpdater() {
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void startServer() 
{
  server.stop();
  server.on("/", webpage);
  server.on("/reboot", reboot);
  server.on("/favicon.ico", HTTP_GET, getFavicon);
  server.begin();
}

void connectWifi() {
  delay(4000);  //Delay needed before calling the WiFi.begin

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(HOSTNAME.c_str());
  WiFi.begin(SSID, PASSWORD, 0, ROUTER_TO_CONNECT_TO);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network " + String(SSID));
  Serial.println(WiFi.localIP());
}

void loop() {
  //Check the current connection status
  if ((WiFi.status() == WL_CONNECTED)) {
    ArduinoOTA.handle();

    returnedAPIString = getHomeAssistantAPIString();

    isKitchenLightsOn = convertJsonToBool(returnedAPIString);
    kitchenLightBrightness = convertJsonToBrightness(returnedAPIString);

    Serial.println(isKitchenLightsOn ? "The kitchen lights are on" : "The kitchen lights are off");

    if (isKitchenLightsOn && !isNovyLightOn) 
    {

      /// The Novy cooking hood lights always start up in full brightness. To dim them down, you need to press and hold the dim button.
      /// It is possible to simulate that button hold by sending the LightOn signal multiple times in a row.
      /// I decided against that, and just leave them off when the other kitchen lights are dimmed below the threshold. (e.g. at night)
      /// Otherwise there would be a flash of light from the cooking hood before it dimmed down to a good level.
      if (kitchenLightBrightness > 200) 
      {
        LightOn();
        isNovyLightOn = true;
        Serial.println("Turned on Novy light");
      }
    }

    if (!isKitchenLightsOn && isNovyLightOn) 
    {
      LightOff();
      isNovyLightOn = false;
      Serial.println("Turned off Novy light");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost connection. Reconnecting...");

    connectWifi();
    startServer();
  }

  server.handleClient();

  delay(REFRESH_TIME);
}

String getHomeAssistantAPIString() {
  HTTPClient http;
  String string = "";
  //Url of the Home Assistant API kitchen lights
  http.begin(HOME_ASSISTANT_LIGHT_URL);
  http.addHeader("Authorization", HOME_ASSISTANT_TOKEN);  //Adds the authorization header
  int httpCode = http.GET();                              //Makes the request

  if (httpCode > 0) {
    string = http.getString();

    Serial.print("WiFi signal strength: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();  //Frees the resources

  return string;
}

/// Checks to which wifi router the ESP32 is connected by comparing the mac addresses of the configured routers in config.h.
/// It returns a friendly name to know which one it is connected to.
String GetFriendlyRouterName() 
{
  /// This compares the first n bytes (where n is the third argument, in this case 6) of
  /// the block of memory pointed by bssid with the first n bytes pointed by bssid_bedroom or bssid_livingroom.
  /// It returns 0 if the two blocks of memory are identical.
  String routerName = "";
  uint8_t* bssid = WiFi.BSSID();

  if (memcmp(bssid, BSSID_BEDROOM, 6) == 0) 
  {
    routerName = "bedroom";
  } 
  else if (memcmp(bssid, BSSID_LIVINGROOM, 6) == 0) 
  {
    routerName = "living room";
  } 
  else 
  {
    routerName = "unknown BSSID";
  }

  return routerName;
}

void getFavicon ()
{
  server.sendHeader("Content-Type", "image/x-icon");
  server.send_P(200, "image/x-icon", (const char*)FAVICON, sizeof(FAVICON));
}

void webpage() {
  int rssi = WiFi.RSSI();
  int signalStrength = map(rssi, -100, -50, 0, 100);
  signalStrength = constrain(signalStrength, 0, 100);

  String routerName = GetFriendlyRouterName();

  String html = R"(
  <html>
    <head>
     <title>)" + String(HOSTNAME)
                + R"(</title>
    <meta http-equiv="refresh" content=")"
                + String(REFRESH_TIME / 1000) + R"(">
    <link rel="icon" type="image/x-icon" href="/favicon.ico">
      <style>
        body {
          background-color: #111111;          
        }
        h1{
          font-family: verdana;
          color: white;
        }
        p {
          font-family: verdana;
          color: white;
        }
        pre{
          color: lightgray;
        }
        .json{
          margin-top: 50px;
          background-color: black;
        }
        .button {
          background-color: blue;
          border: none;
          color: white;
          padding: 15px 32px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 16px;
          margin: 4px 2px;
          cursor: pointer;
        }
      </style>
    </head>
    <body>
      <h1>)" + String(HOSTNAME)
                + R"(</h1>
      <div>
        <p>Connected to )"
                + WiFi.SSID() + R"( ()" + routerName + R"()</p>
        <p>WiFi Signal Strength: )"
                + String(signalStrength) + R"(% ()" + String(rssi) + R"()</p>
        <p>)" + String(isKitchenLightsOn ? "The kitchen lights are on" : "The kitchen lights are off")
                + R"(</p>
        <p>Brightness: )"
                + String(kitchenLightBrightness) + R"(</p>
        <p>Used Heap Size: )"
                + getUsedHeapPercentage() + R"(</p>
        <p>Uptime: )"
                + getUptime() + R"(</p>
        <button class="button" onclick="location.href='/reboot'">Reboot</button>
      </div>
      <div class='json'>
        <pre>)" + beautifyJson(returnedAPIString)
                + R"(</pre>
      </div>
    </body>
  </html>
  )";

  server.send(200, "text/html", html);
}

void reboot() {
  String html = R"(<html>
                  <head>
                  <title>Rebooting...</title>
                    <meta http-equiv='refresh' content='10; url=/'>
                      <style>
                        body {
                          background-color: #111111;          
                        }                      
                        p {
                          font-family: verdana;
                          color: white;
                        }                        
                      </style>
                  </head>
                  <body>
                    <p>Rebooting...</p>
                  </body>
                </html>)";

  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

String getUptime() 
{
  uint64_t timeSinceBoot = esp_timer_get_time();
  uint64_t seconds = timeSinceBoot / 1000000;
  uint64_t minutes = seconds / 60;
  uint64_t hours = minutes / 60;
  uint64_t days = hours / 24;
  seconds = seconds % 60;
  minutes = minutes % 60;
  hours = hours % 24;
  String uptime = String(days) + " days, " + String(hours) + " hours, " + String(minutes) + " minutes, " + String(seconds) + " seconds";

  return uptime;
}


/// Function to calculate the percentage of used memory
String getUsedHeapPercentage() 
{
  // Get the free heap size
  uint32_t freeHeap = esp_get_free_heap_size();

  // Define the total heap size
  uint32_t totalHeap = 320 * 1024;  // 320KB

  // Calculate the used heap size
  uint32_t usedHeap = (freeHeap > totalHeap) ? 0 : (totalHeap - freeHeap);

  // Calculate the percentage of used heap
  float usedHeapPercentage = ((float)usedHeap / totalHeap) * 100;

  // Return the used heap size and percentage as a string
  return String(usedHeap) + " bytes (" + String(usedHeapPercentage) + "%)";
}

String beautifyJson(String jsonString) 
{
  //Create an empty Json object
  StaticJsonDocument<1500> jsonDoc;

  //Load the received data into a JSON object
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (!error) 
  {
    String prettyJson;
    serializeJsonPretty(jsonDoc, prettyJson);
    return prettyJson;
  } 
  else 
  {
    return String("ERROR: Json could not be read");
  }
}

/// Converts the "brightness" property inside the json (returned by Home Assistant) to an int.
/// Returns an int from 0 to 255.
int convertJsonToBrightness(String jsonString) 
{
  //Create an empty Json object
  StaticJsonDocument<1500> jsonDoc;

  //Load the received data into a JSON object
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  // Test if parsing succeeds.
  if (error) 
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return 0;
  }

  JsonObject root = jsonDoc.as<JsonObject>();
  JsonObject attributes = root["attributes"].as<JsonObject>();
  int brightness = attributes["brightness"];

  return brightness;
}


///Converts the "state" property inside the json (returned by Home Assistant) to a bool.
///Returns true if the state is "on", false if it is "off".
bool convertJsonToBool(String jsonString) 
{
  //Create an empty Json object
  StaticJsonDocument<1500> jsonDoc;

  //Load the received data into a JSON object
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  // Test if parsing succeeds.
  if (error) 
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  // Get the state value inside the JSON object
  const char* state = jsonDoc["state"];

  // Convert to boolean
  bool isOn = strcmp(state, "on") == 0;

  return isOn;
}


void PressLight(int channelIndex) 
{
  Serial.print("Pressing light on channel ");
  Serial.println(channelIndex);
  Serial.print((NOVY_DEVICE_CODE[channelIndex] + NOVY_PREFIX + NOVY_COMMAND_LIGHT).c_str());
  Serial.println();
  //010101010111010001
  transmitter.send((NOVY_DEVICE_CODE[channelIndex] + NOVY_PREFIX + NOVY_COMMAND_LIGHT).c_str());
}

void LightOn() 
{
  PressLight(0);
}

void LightOff()
{
  PressLight(0);
}
