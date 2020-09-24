/*
  BME280 I2C DeepSleep MQTT push
*/

#include <BME280I2C.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

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

float temp = 0.0;
float hum = 0.0;
float pres = 0.0;

//////////////////////////////////////////////////////////////////
void setup() {
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

  //Serial.println("Going to Deep sleep for 1 minute...");
  //Depp sleep for 1 minute
  ESP.deepSleep(SLEEP_MINUTES * 60 * 1000000);
}

void publishMeasuredData() {
  if (!client.connected()) {
    reconnect();
  }

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  String placeholder = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"pressure\":" + String(pres) + ",\"battery\":100}";
  Serial.println(placeholder);
  client.publish(MQTT_TOPIC, placeholder.c_str(), true);
}

void reconnect() {
  // Loop until we're reconnected
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
