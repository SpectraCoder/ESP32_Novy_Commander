# ESP32 Novy Commander

The main goal of this project is to control the lights of the Novy Crystal 26050 cooking hood when the Zigbee kitchen lights are turned on or off.
The ESP32 looks at the API of Home Assistant to determine the on or off state of the kitchen lights, and sends a 433mhz signal to the connected 433mhz transmitter.
The ip address of the ESP32 shows a basic status web page with the ability to reboot the device.
 
![ESP32 with a 433mhz transmitter soldered onto it](https://github.com/SpectraCoder/ESP32_Novy_Commander/blob/main/ESP32_Novy_Commander.jpg?raw=true)

For convenience, I soldered the XD-FST FS1000A 433mhz transmitter directly to the ESP32.
By setting pin 22 to HIGH, it powers the transmitter while pin 23 is used to send data.
The pin next to those is ground.

## Usage

To use this in your own setup, make a copy of the [***config.example.h***](https://github.com/SpectraCoder/ESP32_Novy_Commander/blob/main/config.example.h) file. Rename it to ***config.h***.
Change the contents of the file to your own needs.

To be able to access the API of your Home Assistant, you will need to generate a [Long-Lived Access Token](https://www.home-assistant.io/docs/authentication/).
Log in onto your Home Assistant web interface, and click on your profile name. There, on the bottom of the page, you can generate the token. Copy and paste it into the ***config.h*** file.

