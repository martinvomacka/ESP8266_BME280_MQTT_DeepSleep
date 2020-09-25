# ESP8266_BME280_MQTT_DeepSleep
This simple project is to show cheap TEMPERATURE/HUMIDITY/PRESSURE sensor build that:
- Uses cheap and small ESP8266 board
- Uses cheap and small BME280 sensor
- Uses deep-sleep functionality of ESP8266 to minimize the power consumption
- Uses MQTT to publish measured values (further use for various aplications like Smart Home, IFTTT, ...)

## What do you need
- ESP8266 board (this project was tested on **LoLin NodeMCU V3**)
- Bosch BME280 sensor (this project was tested on **GY-BM ME/PM 280** 6-pin board)
- Breadboard or other way to connect GPIO pins
- External Arduino libraries
  - [PubSubClient by Nick O'Leary](https://github.com/knolleary/pubsubclient/)
  - [BME280 by finitespace](https://github.com/finitespace/BME280)
- MQTT server to push measured values

## Wiring diagram
**Pins D0 (ESP8266 WAKE) and RST must be connected together so the board can wake up from deep-sleep.**
![Wiring diagram](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/wiring_diagram.jpg)

## To-Do / Further plans
- [x] Create wiring diagram
- [ ] Test on different ESP8266 boards
- [ ] Measure power consumption for deep-sleep vs. sleep time combinations
- [ ] Compare measurements between different board vendors
- [ ] Create interactive "web inital setup" - SoftAP portal for initial setup (SSID, WiFi password, MQTT server IP, MQTT topic, sleep time)

## Photos of used HW
###### BME280 sensor (bought online) - this is 3.3V version! Be careful about this, there is also 5V version.
Bottom view:
![Bottom view of BME280](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/BME280_bottom.jpg)
Top view:
![Top view of BME280](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/BME280_top.jpg)
###### LoLin NodeMCU V3 - I have borrowed this board since I'm waiting for ordered Wemos D1 Mini.
![LoLin NodeMCU V3](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/NodeMCU_V3.jpg)