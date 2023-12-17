/// Make a copy of this file, and rename it to "config.h" to make it work with the EPS32_Novy_Commander.ino script.
/// Change the variables in ths file to your needs. 
const String HOSTNAME = "ESP32 Novy Commander";
const char* SSID = "Your wifi name";
const char* PASSWORD =  "Your password";
const int REFRESH_TIME = 1000;  ///milliseconds

/// For convenience, I soldered the 433mhz transmitter directly to the ESP32.
/// By setting pin 22 to HIGH, it powers the transmitter while pin 23 is used to send data.
/// The pin next to those is ground.
const int TRANSMIT_433MHZ_PIN = 23;
const int POWER_433MHZ_PIN = 22;

/// In my own home I have two wifi routers using the same SSID (wifi name). 
/// The ESP32 isn't very good at picking the strongest signal by itself reliably, so I needed a way to tell it which router to pick.
/// By defining the Mac address of the router in the ROUTER_TO_CONNECT_TO variable, it only connects to that router.
/// I made two variables for debugging purposes.
const uint8_t BSSID_BEDROOM[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /// Mac Address of the bedroom router to connect to
const uint8_t BSSID_LIVINGROOM[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /// Mac Address of the living room router to connect to

const uint8_t* ROUTER_TO_CONNECT_TO = BSSID_BEDROOM; /// Selects the router to connect to

/// Create a token in Home Assistant to access its API. Be sure to add the word "Bearer " (with a space after it) in front of the token.
const char* HOME_ASSISTANT_TOKEN = "Bearer kajhfsdklfasglksajglkasglaslglksadjglkjdsglkjagjdgbflbkdsfjdlsdfasdlkfhghasjfgeyrtaefhjgsdkjhfgsjfgkaekgfasjfbsdvnbbvzvbshdkgfgjsdalgdslkaglksjdasdgagagaga";
const char* HOME_ASSISTANT_LIGHT_URL = "http://192.168.1.7:80/api/states/light.examplelight"; /// Url of the Home Assistant API lights you want to use.

/// The 433mhz Novy device codes I borrowed from https://github.com/abelgomez/rf-mqtt-bridge/blob/master/src/NovyController.cpp
/// Programmable code (from 1 to 10), see 4 first bits above
static const String NOVY_DEVICE_CODE[] = {
    "0101",
    "1001",
    "0001",
    "1110",
    "0110",
    "1010",
    "0010",
    "1100",
    "0100",
    "1000",
};

/// Prefix (next 4 bits, not sure what it means, if it is always fixed, or if it is part of the command code)
static const String NOVY_PREFIX = "0101";

/// Commands to be sent (code + prefix + command)
static const String NOVY_COMMAND_LIGHT = "0111010001";
static const String NOVY_COMMAND_POWER = "0111010011";
static const String NOVY_COMMAND_PLUS =  "0101";
static const String NOVY_COMMAND_MINUS = "0110";
static const String NOVY_COMMAND_NOVY =  "0100";