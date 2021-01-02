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

## Wiring diagram (LoLin NodeMCU V3)
**Pins D0 (ESP8266 WAKE) and RST must be connected together so the board can wake up from deep-sleep.**
![Wiring diagram](https://github.com/martinvomacka/ESP8266_BME280_MQTT_DeepSleep/blob/master/photos/wiring_diagram.jpg)

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
  - ESP-12F - *aditional components required (minimum 3x 10k? resistor) + tiny precise soldering*
- [x] Measure power consumption for deep-sleep vs. sleep time combinations
- [x] Compare measurements between different board vendors
- [ ] Create interactive "web inital setup" - SoftAP portal for initial setup (SSID, WiFi password, MQTT server IP, MQTT topic, sleep time)  
- [ ] Plan a design compact case to be able to print it on 3D printer

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