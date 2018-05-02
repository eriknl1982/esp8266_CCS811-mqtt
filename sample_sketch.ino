/***************************************************************************
  Sample sketch for using a CCS811 CO2 sensor with a ESP8266 sensor and sending the result over MQTT once a minute. 
  Written by Erik Lemcke, using the following samples:
  
  https://learn.adafruit.com/adafruit-ccs811-air-quality-sensor/arduino-wiring-test, sample and library by adafruit 
  https://www.home-assistant.io/blog/2015/10/11/measure-temperature-with-esp8266-and-report-to-mqtt/, home assistant mqqt by Paulus Schoutsen
  
  wiring:
  ESP --> ccs811
  4       SDA
  5       SCL 
  gnd     gnd
  gnd     wake
  vcc     vcc
  
 ***************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Adafruit_CCS811.h"

#define wifi_ssid "<YOUR WIFI SSID>"
#define wifi_password "<YOUR WIFI PASSWORD>"

#define mqtt_server "<YOUR MQTT SERVER IP>"
#define mqtt_user "<YOUR MQTT USERNAME>"
#define mqtt_password "<YOUR MQTT PASSWORD>"

#define co2_topic "ccs811/co2"

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_CCS811 ccs;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  Serial.println("Startup");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  unsigned long previousMillis = millis();

  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  delay(500);
}

long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  //send a meaage every minute
  if (now - lastMsg > 60 * 1000) {
    lastMsg = now;
   

       if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print("ppb   Temp:");
      Serial.println(temp);

      client.publish(co2_topic, String(ccs.geteCO2()).c_str(), true);
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }


      
    
      
    }
  }
}
