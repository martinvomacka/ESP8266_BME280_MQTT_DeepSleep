/*
    ESP8266 BME280 MQTT DeepSleep demo
    
    Make sure you input proper values for:
    WIFI_SSID
    WIFI_PWD
    MQTT_SERVER
    MQTT_TOPIC
    SLEEP_MINUTES
    
    This project was tested on following HW:
    LoLin NodeMCU v3
    Bosch BME280 3.3V sensor (GY-BM ME/PM 280 board - 6 pins)
    
    Used external libraries:
    PubSubClient by Nick O'leary - https://github.com/knolleary/pubsubclient/
    BME280 by finitespace - https://github.com/finitespace/BME280
*/

#include <BME280I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>
#include <PubSubClient.h>

//Define serial speed
#define SERIAL_BAUD 115200
//Define your WiFi SSID to connect to
#define WIFI_SSID "your_wifi_SSID"
//Define password to your WiFi
#define WIFI_PWD "your_wifi_PASSWORD"
//Define IP address of your MQTT server
#define MQTT_SERVER "MQTT_IP_address"
//Define target MQTT topic to push values into
#define MQTT_TOPIC "MQTT_target_topic"
//Define for how long should ESP8266 sleep before tanking new measure
#define SLEEP_MINUTES 1
//Global Wifi handler
WiFiClient espClient;
//Global MQTT client handler
PubSubClient client(espClient);
//Global BME280 sensor handler
BME280I2C bme;
//Set the temperature unit
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
//Set the pressure unit
BME280::PresUnit presUnit(BME280::PresUnit_hPa);
//Global variables for measured values
float temp = 0.0;
float hum = 0.0;
float pres = 0.0;
//FW version for OTA updates
const int FW_VERSION = 1000;
//FW download URL for OTA updates
const char* FWURL = "http://YOUR_WEBSERVER_FW_PATH/";

/*
    For deep-sleep to work properly, everything must happen in the setup()
    function including the last step - going back to deep-sleep.
    
    !!!For deep-sleep to work correctly, it is necesary to connect RST pin with
    WAKE pin on your ESP8266 board!!!
    
    BME280 sensor is connected via I2C. It is set to standard GPIO pins:
    SCL - GPIO4
    SDA - GPIO5
    VCC/GND - any available 3.3V/5V and GND pins on your board
    !!!Warning - selected voltage depends on the BME280 variant (3.3V or 5V)!!!
    
    For quick overwiev of (most common) boards pinout take a look on this link:
    https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
*/
void setup() {
  //Setup serial communication
  Serial.begin(SERIAL_BAUD);
  while (!Serial) {} // Wait

  //Setup WiFi connection
  WiFi.hostname("Kitchen-WS"); // Kitchen - weather station
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  //Check for OTA FW update - UNCOMMENT TO USE THIS FEATURE
  //checkForUpdates();

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

  //Deep-sleep for defined time
  ESP.deepSleep(SLEEP_MINUTES * 60 * 1000000);
}

/*
    This function checks server WWW path from fwServerUrl via HTTP GET to check for available FW updates.
    This is done in 2 steps:
    1) [board-MAC-address].verison file is checked on the server.
      Verison file contains only one line - numeric value of FW version on the server.
      If the server FW version is higher than board FW version, OTA update will begin
    2) OTA update downloads [board-MAC-address].bin file from server and starts the update.
      Board is automaticaly rebooted after the OTA update.
*/
void checkForUpdates() {
  //Get board MAC address
  String mac = WiFi.macAddress();
  //Remove ":" from MAC address string (char ":" is not allowed in filenames)
  mac.replace(":","");
  //Prepare URL to check FW version on server for this board
  String boardUrl = String(fwServerUrl);
  boardUrl.concat(mac);
  //Preapre separate String verisonUrl to be able tu use boardUrl to dowload .bin if needed in next steps
  String versionUrl = boardUrl;
  versionUrl.concat(".version");
  //Create new HTTP connection using the version URL
  HTTPClient httpClient;
  httpClient.begin(versionUrl);
  //GET version from URL
  int httpRetCode = httpClient.GET();
  if(httpRetCode == 200) {
    //Successfully found and .version file on server - read the version number as string
    String serverFwVersion = httpClient.getString();
    //Convert String number to INT
    int newVersion = serverFwVersion.toInt();
    //Compare current version with server version
    if(newVersion > FW_VERSION) {
      Serial.print("Server has new FW available - starting update to FW version: ");
      Serial.println(serverFwVersion);
      //Preapre separate String binUrl to download the FW binary
      String binUrl = boardUrl;
      binUrl.concat(".bin");
      //Start the FW update via OTA using binUrl
      ESPhttpUpdate.update(binUrl);
    }
    else {
      Serial.print("Board is running latest FW version: ");
      Serial.println(FW_VERSION);
    }
  }
  else {
    Serial.print("Error during HTTP GET:");
    Serial.println(httpRetCode);
  }
  httpClient.end();
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
