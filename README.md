# ESP8266_BME280_MQTT_DeepSleep
This simple project is to show cheap TEMPERATURE/HUMIDITY/PRESSURE sensor build that:
- Uses cheap and small ESP8266 board
- Uses cheap and small BME280 sensor
- Uses deep-sleep functionality of ESP8266 to minimize the power consumption
- Uses MQTT to publish measured values (further use for various aplications like Smart Home, IFTTT, ...)

## What do you need
- ESP8266 board (this project was tested on LoLin NodeMCU V3)
- Bosch BME280 sensor (this project was tested on GY-BM ME/PM 280 6 pins board)
- Breadboard or other way to connect GPIO pins
- External Arduino libraries
  - [PubSubClient by Nick O'Leary](https://github.com/knolleary/pubsubclient/)
  - [BME280 by finitespace](https://github.com/finitespace/BME280)
- MQTT server to push measured values