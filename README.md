# ESP8266_BME280_MQTT_DeepSleep
This simple project is to show cheap TEMPERATURE/HUMIDITY/PRESSURE sensor build that:
- Uses cheap and small ESP8266 board
- Uses cheap and small BME280 sensor
- Uses deep-sleep functionality of ESP8266 to minimize the power consumption
- Uses MQTT to publish measured values (further use for various aplications like Smart Home, IFTTT, ...)
- **Has SoftAP config mode** to setup all values (WiFi SSID, WiFi password, MQTT server IP, MQTT topic, sleep time, OTA URL (optional))
- **Utilizes HW switch to select mode** (standard/config). Default switch PIN is set to D6 (GPIO12) - **connect GPIO12 to GND to enter config mode**

## What do you need
- ESP8266 board - I use ESP-12F (alse tested on **LoLin NodeMCU V3, Wemos D1 mini**)
- Bosch BME280 sensor (this project was tested on **GY-BM ME/PM 280** 6-pin board)
- Breadboard or other way to connect GPIO pins (soldering)
- External Arduino libraries
  - [PubSubClient by Nick O'Leary](https://github.com/knolleary/pubsubclient/)
  - [BME280 by finitespace](https://github.com/finitespace/BME280)
- MQTT server and topic to push measured values
- *[OPTIONAL] OTA server to use OTA fatures (needs URL where to look for new FW)*

## Wiring diagram (LoLin NodeMCU V3)
**Pins D0 (ESP8266 WAKE) and RST must be connected together so the board can wake up from deep-sleep.**
**Connect D6 to GND to enter config mode**
![Wiring diagram](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/wiring_diagram.jpg)

## SoftAP
Simple webpage to enter all needed values:
- **Enter SSID** - WiFi SSID that will be used to connect to during standard mode
- **Enter password** - WiFi password that will be used to connect to during standard mode
- **Measurement interval [minutes]** - time used for deep sleep in minutes (takes value 1-60)
- **MQTT server IP address** - IP for the MQTT server
- **MQTT topic path** - topic to store values into
- **Use OTA updates?** - Radio button to select whether to use OTA updates or not (if **No** selected, the URL text field will become inactive)
- **SAVE** - stores the values into EEPROM and refreshes the page

![SoftAP webpage](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/softap_web.png)

## Measured consumption
Below are my measurements on different setups/boards that are all running exactly the same code.

- **LoLin NodeMCU V3** - Direct Li-Po battery to NodeMCU +3V/GND
  - Boot/running ~74mA with peaks to 85mA
  - Deep sleep ~4.3mA
- **Wemos D1 mini** - Direct Li-Po battery to Wemos +3V/GND
  - Boot/running ~68mA with peaks to 77mA
  - Deep sleep ~477µA
- **Wemos D1 mini** - Li-Po battery connected via battery shield to Wemos
  - Boot/running ~106mA with peaks to 121mA
  - Deep sleep ~326µA
- **ESP-12F** - Direct Li-Po battery to ESP-12F +3V/GND
  - Boot/running ~68mA with peaks to 76mA
  - **Deep sleep ~18.9µA**
- **ESP-12F** - Li-Po battery connected via LDO step-down (HX4002-3.3)
  - Boot/running ~137mA with peaks to 155mA
  - Deep sleep ~377µA

## To-Do / Further plans
- [x] Create wiring diagram
- [x] Test on different ESP8266 boards
  - LoLin NodeMCU V3
  - Wemos D1 mini
  - ESP-12F - *aditional components required (power supply, resistors, etc...) + tiny precise soldering*
- [x] Measure power consumption for deep-sleep vs. sleep time combinations
- [x] Compare measurements between different board vendors
- [-] Implement options for MQTT
  - [x] Authentication (right now it uses public access to MQTT)
  - [x] Port (right now it uses default port 1883)
  - [ ] Secure conection (???)
- [x] Added OTA update feature - all credits to Erik H. Bakke and his [tutorial](https://www.bakke.online/index.php/2017/06/02/self-updating-ota-firmware-for-esp8266/) - please follow this link to undestand properly how to setup OTA (webserver to store .version and .bin files needed)
- [x] Create interactive "web inital setup" - SoftAP portal for setup (SSID, WiFi password, MQTT server IP, MQTT topic, sleep time, OTA URL (optional))
  - [x] Add www radio button for optional MQTT authentication
  - [x] Add switch or jumper to be able to enter this mode on demand (jumper/switch on - boot to SoftAP and publish setting website... turn off and get back to measuring and deep sleep cycles)
- [ ] Implement secure method to post values on webpage instead of simple HTTP POST
- [ ] Plan a design compact case to be able to print it on 3D printer
- [x] Design and make custom PCB

## Photos of used HW
###### BME280 sensor (bought online) - this is 3.3V version! Be careful about this, there is also 5V version.
Bottom view:
![Bottom view of BME280](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/BME280_bottom.jpg)
Top view:
![Top view of BME280](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/BME280_top.jpg)
###### LoLin NodeMCU V3
![LoLin NodeMCU V3](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/NodeMCU_V3.jpg)
###### Wemos D1 mini
![Wemos D1 mini](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/Wemos_D1_mini.jpg)
###### Wemos D1 mini + battery shield
![Wemos D1 mini + battery shield](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/Wemos_D1_mini_BS.jpg)
###### ESP-12F
![ESP-12F](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/ESP-12F.jpg)
###### ESP-12F in development board (spring contacts)
![ESP-12F in devboard](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/ESP-12F_in_devboard.jpg)