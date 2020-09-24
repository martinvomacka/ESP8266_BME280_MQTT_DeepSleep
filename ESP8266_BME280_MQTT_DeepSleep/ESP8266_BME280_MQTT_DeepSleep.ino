/*
    BME280 I2C DeepSleep MQTT push
*/

#include <BME280I2C.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

// define serial speed
#define SERIAL_BAUD 115200
// define your WiFi SSID to connect to
#define WIFI_SSID "your_wifi_SSID"
// define password to your WiFi
#define WIFI_PWD "your_wifi_PASSWORD"
// define IP address of your MQTT server
#define MQTT_SERVER "your_MQTT_server_IP"
// define target MQTT topic to push values into
#define MQTT_TOPIC "MQTT_target_topic"
// define for how long should ESP8266 sleep before tanking new measure
#define SLEEP_MINUTES 1
// global Wifi handler
WiFiClient espClient;
// global MQTT client handler
PubSubClient client(espClient);
// global VME280 sensor handler
BME280I2C bme;
// set the temperature unit
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
// set the pressure unit
BME280::PresUnit presUnit(BME280::PresUnit_hPa);
// global variables for measured values
float temp = 0.0;
float hum = 0.0;
float pres = 0.0;

/*
    For deep-sleep to work properly, everything must happen in the setup()
    function including the last step - going back to deep-sleep.
    
    !!!For deep-sleep to work correctly, it is necesary to connect RST pin with
    WAKE pin on your ESP8266 board!!!
    
    BME280 sensor is connected via I2C. It is set to standard GPIO pins:
    SCL - GPIO4
    SDA - GPIO5
    VCC/GND - any available 3.3V and GND pins on your board
    
    For quick overwiev of (most common) board pinouts take a look on this link:
    https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
*/
void setup() {
  //Setup serial communication
  Serial.begin(SERIAL_BAUD);
  while (!Serial) {} // Wait

  //Setup WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //Setup MQTT server
  client.setServer(MQTT_SERVER, 1883);

  //Setup I2C connection for the sensor
  Wire.begin();
  while (!bme.begin())
  {
    delay(500);
  }

  //Publish measured data over MQTT
  publishMeasuredData();
  delay(1000);

  //Serial.println("Going to deep-sleep for 1 minute...");
  //Deep-sleep for defined time
  ESP.deepSleep(SLEEP_MINUTES * 60 * 1000000);
}

/*
    This function has 3 steps:
    1) wait for successfull connection to MQTT server
    2) take measurement from the BME280 sensor
    3) push the measured data to the target MQTT topic
*/
void publishMeasuredData() {
  //Wait for sccessfull connection to MQTT
  if (!client.connected()) {
    reconnect();
  }
  //Read values from the sensor
  bme.read(pres, temp, hum, tempUnit, presUnit);
  //Prepare JSON string to push to MQTT server/topic
  //Curent format of JSON string is: '{"temperature":TEMP_VALUE,"humidity":HUMIDITY_VALUE,"pressure":PRESSURE_VALUE}'
  String placeholder = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"pressure\":" + String(pres) + "}";
  //String placeholder = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"pressure\":" + String(pres) + ",\"battery\":100}"; - prep for battery capacity measurement
  //Debug print of JSON string to serial
  //Serial.println(placeholder);
  //Publish JSON string to the MGTT server/topic
  client.publish(MQTT_TOPIC, placeholder.c_str(), true);
}

/*
    Funciton to make sure we are successfully connected to MQTT server
*/
void reconnect() {
  //Loop until connected to MQTT
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
    }
    else {
      delay(1000);
    }
  }
}

void loop() {
}
