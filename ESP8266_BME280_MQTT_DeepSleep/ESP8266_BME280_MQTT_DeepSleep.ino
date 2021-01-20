/*
    ESP8266 BME280 MQTT DeepSleep demo
    
    Has its own SoftAP mode to setup all values + OTA can be used for future FW updates.
    ESP8266 onboard LED used for mode indication - read comment on setup() function.
    Connecting D6 (GPIO12) to GND is used for mode selection - read comment on setup() function.
    Both LED and SWITCH can be changed in the code definition
    
    This project was tested on following HW:
    LoLin NodeMCU v3
    Wemos D1 Mini
    ESP-12F
    Bosch BME280 3.3V sensor (GY-BM ME/PM 280 board - 6 pins)
    
    Used external libraries:
    PubSubClient by Nick O'leary - https://github.com/knolleary/pubsubclient/
    BME280 by finitespace - https://github.com/finitespace/BME280
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <BME280I2C.h>

//Use onboard LED on ESP-12F for simple mode indication
#define LED D4
//USE GPIO12 as mode select - leave GPIO12 disconnected = standard mode (measure)
//USE GPIO12 as mode select - connect GPIO12 to GND = config mode (SoftAP to setup values)
#define SWITCH D6
//Define serial speed
#define SERIAL_BAUD 115200
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
const int FW_VERSION = 1002;
//Variables definition for EEPROM
char ssid[32] = "";
char password[32] = "";
char mqtt_ip[20] = "";
char mqtt_topic[128] = "";
char ota_url[128] = "";
int interval;
int use_ota;
//Global bools for setup and loop control
bool exec_mode;
bool creds;
//SoftAP WLAN settings - chose whatever you want
const char *softAP_ssid = "ESP-MQTT";
const char *softAP_password = "12345678";
//SoftAP network parameters - chose whatever network settings you want - use the IP to connect to SoftAP website
IPAddress softAP_ip(192, 168, 4, 1);
IPAddress softAP_netmask(255, 255, 255, 0);
//Global webserver handler defined to listen on port 80 (standard HTTP port)
ESP8266WebServer server(80);

/**
  * Loads all variables from EEPROM into program.
  * @param[out] bool Return TRUE if EEPROM contained data and load was successful, else return FALSE
  */
bool loadCredentials() {
  //Setup inital address to read from
  EEPROM.begin(512);
  //Read WLAN SSID
  EEPROM.get(0, ssid);
  //Read WLAN password
  EEPROM.get(0+sizeof(ssid), password);
  //Read measurement interval
  EEPROM.get(0+sizeof(ssid)+sizeof(password), interval);
  //Read MQTT server IP address
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(interval), mqtt_ip);
  //Read MQTT topic
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip), mqtt_topic);
  //Read OTA server URL
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic), ota_url);
  //Read indicator whether use OTA or not
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic)+sizeof(ota_url), use_ota);
  char ok[2+1];
  //Read last 2 bytes as verification we reached end of all stored values
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic)+sizeof(ota_url)+sizeof(use_ota), ok);
  EEPROM.end();
  //Compare last 2 read bytes to verify all the contents were read properly (last 2 bytes must contain string "OK") if not, set all variables null
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    mqtt_ip[0] = 0;
    mqtt_topic[0] = 0;
    ota_url[0] = 0;
    interval = 0;
    use_ota = 0;
    return false;
  }
  else {
    return true;
  }
}

/** 
  * Stores all variables from program to EEPROM.
  */
void saveCredentials() {
  //Setup inital address to write to
  EEPROM.begin(512);
  //Store WLAN SSID
  EEPROM.put(0, ssid);
  //Store WLAN password
  EEPROM.put(0+sizeof(ssid), password);
  //Store measurement interval
  EEPROM.put(0+sizeof(ssid)+sizeof(password), interval);
  //Store MQTT server IP address
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(interval), mqtt_ip);
  //Store MQTT topic
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip), mqtt_topic);
  //Store OTA server URL
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic), ota_url);
  //Store 0 or 1 to indicate whether use OTA or not
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic)+sizeof(ota_url), use_ota);
  //Store "OK" as verification we reached end of all stored values
  char ok[2+1] = "OK";
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(interval)+sizeof(mqtt_ip)+sizeof(mqtt_topic)+sizeof(ota_url)+sizeof(use_ota), ok);
  //ESP8266 - need to .commit() and .end() to write the contents to FLASH
  EEPROM.commit();
  EEPROM.end();
}

/** 
  * Sends HTTP 404 to client invalid request.
  */
void handleNotFound() {
  //Build the message as simple plain text to be send to user as error webpage
  String message = "HTTP 404: The requested URL ";
  //Print what URI user tried to access
  message += server.uri();
  message += " was not found on this server.\n";
  //Setup webpage header
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  //Send webpage to user
  server.send ( 404, "text/plain", message );
}

/** 
  * Prints whole webpage of SoftAP portal.
  * This is just a HTML code wrapped into the C program to be able present webpage FORM and all variables to user.
  * More explanation to this HTML code in README.md
  */
void printWebPage() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.sendContent(
      "<!DOCTYPE HTML>"
      "<html>"
      "<head>"
      "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
      "<title>TEST setup portal</title>"
      "<style>"
      "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
      "</style>"
      "</head>"
      "<body>"
      "<script type=\"text/javascript\">"
      "function EnableDisableTextBox(){var chkYes = document.getElementById(\"otaYes\");"
      "var txtPassportNumber = document.getElementById(\"otaText\");"
      "otaText.disabled = chkYes.checked ? false : true;"
      "if (!otaText.disabled) {otaText.focus();}}"
      "</script>"
      "<h1>TEST setup portal</h1>"
      "<form action=\"/\" method=\"post\">"
      "<p>"
      "<label for=\"ssid\">Enter SSID:</label><br />"
      "<input type=\"text\" name=\"ssid\" value=\""
    );
    server.sendContent(String(ssid));
    server.sendContent(
      "\" required><br />"
      "<label for=\"password\">Enter password:</label><br />"
      "<input type=\"password\" name=\"password\" value=\""
    );
    server.sendContent(String(password));
    server.sendContent(
      "\" required><br />"
      "<label for=\"password\">Measurement interval [minutes]:</label><br />"
      "<input type=\"number\" pattern=\"[0-9]*\" name=\"interval\" min=\"1\" max=\"60\" value=\""
    );
    server.sendContent(String(interval));
    server.sendContent(
      "\" required><br />"
      "<label for=\"mqtt-ip\">MQTT server IP address:</label><br />"
      "<input type=\"text\" name=\"mqtt-ip\" minlength=\"7\" maxlength=\"15\" size=\"15\" pattern=\"^((\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$\" value=\""
    );
    server.sendContent(String(mqtt_ip));
    server.sendContent(
      "\"><br />"
      "<label for=\"mqtt-topic\">MQTT topic path:</label><br />"
      "<input type=\"text\" name=\"mqtt-topic\" maxlength=\"127\" value=\""
    );
    server.sendContent(String(mqtt_topic));
    server.sendContent(
      "\"><br />"
      "Use OTA updates?<br />"
    );
    if(use_ota) {
      server.sendContent(
        "<input type=\"radio\" id=\"otaYes\" name=\"ota-radio\" value=\"1\" checked=\"checked\" onclick=\"EnableDisableTextBox()\"><label for=\"otaYes\">Yes</label><br />"
        "<input type=\"radio\" id=\"otaNo\" name=\"ota-radio\" value=\"0\" onclick=\"EnableDisableTextBox()\"><label for=\"otaNo\">No</label><br />"
        "<input type=\"text\"  id=\"otaText\" name=\"ota-url\" size=\"32\" maxlength=\"127\" value=\""
      );
    }
    else {
      server.sendContent(
        "<input type=\"radio\" id=\"otaYes\" name=\"ota-radio\" value=\"1\" onclick=\"EnableDisableTextBox()\"><label for=\"otaYes\">Yes</label><br />"
        "<input type=\"radio\" id=\"otaNo\" name=\"ota-radio\" value=\"0\" checked=\"checked\" onclick=\"EnableDisableTextBox()\"><label for=\"otaNo\">No</label><br />"
        "<input type=\"text\"  id=\"otaText\" disabled name=\"ota-url\" size=\"32\" maxlength=\"127\" value=\""
      );
    }
    server.sendContent(String(ota_url));
    server.sendContent(
      "\"><br />"
      "<input type=\"submit\" value=\"SAVE\">"
      "</p>"
      "</form>"
      "</body>"
      "</html>"
    );
    server.client().stop(); // Stop is needed because we sent no content length
}

/** 
  * Handels HTTP requests on root level and stores data posted from the webpage form.
  */
void handleRoot() {
  //If HTTP request has more than 1 arguments, it means that data were send via HTTP POST from form
  if (server.args()>=2) {
    //Process all args and save them to variables
    server.arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
    server.arg("password").toCharArray(password, sizeof(ssid) - 1);
    server.arg("mqtt-ip").toCharArray(mqtt_ip, sizeof(mqtt_ip) - 1);
    server.arg("mqtt-topic").toCharArray(mqtt_topic, sizeof(mqtt_topic) - 1);
    if(server.arg("ota-radio").toInt() == 1) {
      use_ota = 1;
      server.arg("ota-url").toCharArray(ota_url, sizeof(ota_url) - 1);
    }
    else {
      use_ota = 0;
      String("http://server/OTA/").toCharArray(ota_url, sizeof(ota_url) - 1);
    }
    interval = server.arg("interval").toInt();
    //Store all variables to EEPROM
    saveCredentials();
    //Refresh webpage
    printWebPage();
  }  
  //If HTTP request has zero or none arguments, it is root access and we will only display webpage
  else { //access to
    printWebPage();
  }
}

/**
  * This function has 3 steps:
  * 1) wait for successfull connection to MQTT server
  * 2) take measurement from the BME280 sensor
  * 3) push the measured data to the target MQTT topic
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
  //String placeholder = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"pressure\":" + String(pres) + "}";
  String placeholder = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"pressure\":" + String(pres) + ",\"battery\":100}"; //- prep for battery capacity measurement
  
  //Debug print of JSON string to serial
  //Serial.println(placeholder);
  
  //Publish JSON string to the MGTT server/topic
  client.publish(mqtt_topic, placeholder.c_str(), true);
}

/**
  * Function to make sure we are successfully connected to MQTT server
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

/**
  * This function checks server WWW path from fwServerUrl via HTTP GET to check for available FW updates.
  * This is done in 2 steps:
  * 1) [board-MAC-address].verison file is checked on the server.
  *   Verison file contains only one line - numeric value of FW version on the server.
  *   If the server FW version is higher than board FW version, OTA update will begin
  * 2) OTA update downloads [board-MAC-address].bin file from server and starts the update.
  *   Board is automaticaly rebooted after the OTA update.
  */
void checkForUpdates() {
  //Get board MAC address
  String mac = WiFi.macAddress();
  //Remove ":" from MAC address string (char ":" is not allowed in filenames)
  mac.replace(":","");
  //Prepare URL to check FW version on server for this board
  String boardUrl = String(ota_url);
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

/**
  * Board setup - decides wether run in standard mode or config mode.
  * Config mode:
  * If D6 (GPIO12) is connected to GND, enters config mode (SoftAP).
  * If this is first board run, ESP8266 onboard blue LED will blink slowly indicating that default values were generated and board needs one reset (unplug the power or use RST button). During this phase, SoftAP is not created.
  * If this is not the first run and there are stored data on EEPROM, SoftAP will start and ESP8266 onboard blue LED will be turned ON for the whole time.
  * To exit config mode, disconnect D6 (GPIO12) from GND and reset the board (unplug the power or use RST button).
  * Standard mode:
  * If this is first board run, ESP8266 onboard blue LED will blink rapidly indicating that inital config mode must be run to at least generate default values.
  * If this is not the first run, the board will load all values from EEPROM and proceed with standard measurement and MQTT publishing process.
  */
void setup(void)
{
  Serial.begin(74880);
  while (!Serial) {} // Wait for serial init
  pinMode(LED, OUTPUT); // Initialize the LED pin as an output
  pinMode(SWITCH, INPUT); // Initialize the SWITCH pin as an input
  exec_mode = digitalRead(SWITCH); //Get SWITCH status
  creds = loadCredentials(); //Load values from EEPROM
  if(!exec_mode) {
    Serial.println();
    Serial.println("Config mode!");
    digitalWrite(LED, LOW);
    if(creds) {
      Serial.println("Found stored credentials, using them.");
      WiFi.softAPConfig(softAP_ip, softAP_ip, softAP_netmask);
      WiFi.softAP(softAP_ssid, softAP_password);
      delay(500);
      
      server.on("/", handleRoot);  
      server.onNotFound(handleNotFound);
      server.begin();
    }
    else {
      Serial.println("NO CREDENTIALS, generating defaults. PLEASE RESTART DEVICE!");
      String("SSID").toCharArray(ssid, sizeof(ssid) - 1);
      String("password").toCharArray(password, sizeof(password) - 1);
      String("000.000.000.000").toCharArray(mqtt_ip, sizeof(mqtt_ip) - 1);
      String("default/topic").toCharArray(mqtt_topic, sizeof(mqtt_topic) - 1);
      String("http://server/OTA/").toCharArray(ota_url, sizeof(ota_url) - 1);
      interval = 1;
      use_ota = 0;
      saveCredentials();
    }
  }
  else {
    if(creds) {
      Serial.println();
      Serial.println("Standard mode...");
      digitalWrite(LED, HIGH);
      WiFi.hostname("Kitchen-WS"); // Kitchen - weather station
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

      //Check for OTA FW update
      if(use_ota) {
        checkForUpdates();
      }
      
      //Setup MQTT server
      client.setServer(mqtt_ip, 1883);
    
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
      ESP.deepSleep(interval * 60 * 1000000);
    }
    else {
      Serial.println();
      Serial.println("Standard mode... NO CREDS! Switch to config mode to generate defaults!");
    }
  }
}

/**
  * Loop function used only: 
  * 1) during config mode (to process all connected clients HTTP requests)
  * 2) during standard mode with no stored credentials (rapid LED blinking)
  */
void loop(void)
{
  if (!exec_mode) {
    if (creds) {
      digitalWrite(LED, LOW);
    }
    else {
      digitalWrite(LED, LOW); // Turn the LED on
      delay(1000); //Wait one second
      digitalWrite(LED, HIGH); // Turn the LED off
      delay(1000); //Wait one seconds
    }
    server.handleClient();
  }
  else {
    if (!creds) {
      digitalWrite(LED, LOW); // Turn the LED on
      delay(100); // Wait 1/10 second
      digitalWrite(LED, HIGH); // Turn the LED off
      delay(100); // Wait 1/10 seconds
    }
  }
}
