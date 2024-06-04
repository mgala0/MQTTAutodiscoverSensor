#include <Arduino.h>
#include <WiFi.h>
#include "WiFiCred.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define BAUDRATE 115200
#define MQTT_BROKER_IP "192.168.254.25"
//test idx for nonautodiscover: 7
//mosquitto password without special characters !!!
#define HOSTNAME "ESP_LIVE_MINI"
#define MQTT_CLIENT_NAME "ESP_LIVE_MINI"
uint8_t svalue = 20; //fake sensor value to send

WiFiClient ESP32Client;
PubSubClient MQTTClient(ESP32Client);

void WiFiConnect(const char *ssid, const char *password);
void MQTTConnect(const char* server, const char *user, const char *passwd);
void MQTTSendMessage (String topic, String message);



void setup()
{
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);
  WiFiConnect(SSID, WIFI_PASSWD);
  MQTTConnect(MQTT_BROKER_IP, MQTT_USER, MQTT_PASSWD);
  //MQTTSendMessage(message);
}

void loop()
{
  String topic = "domoticz/in"; 
  String message;
  JsonDocument MQTT_Message;
  MQTT_Message["idx"] = 7;
  MQTT_Message["nvalue"] = 0;
  MQTT_Message["svalue"] = String(svalue);
  svalue++;
  if (svalue>35) svalue = 20;
  serializeJson(MQTT_Message, message); 
  Serial.println(message);
  MQTTSendMessage(topic, message);
  Serial.println("KABOOM!!!");
  delay(2000);

}

void WiFiConnect(const char *ssid, const char *password)
{
  WiFi.setHostname(HOSTNAME);    //define hostname FIRST
  WiFi.mode(WIFI_STA);            //then mode
  WiFi.begin(ssid, password);
  Serial.println(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print(F("Connected to WiFi network with IP Address: "));
  Serial.println(WiFi.localIP());
  delay(1000);
}

void MQTTConnect(const char* server, const char *user, const char *passwd)
{
  MQTTClient.setServer(server, 1883);
  while (!MQTTClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (MQTTClient.connect(MQTT_CLIENT_NAME, user, passwd)) 
    {
      Serial.println("connected");
    }
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(MQTTClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MQTTSendMessage (String topic, String message)
{
  if (!MQTTClient.connected()) MQTTConnect(MQTT_BROKER_IP, MQTT_USER, MQTT_PASSWD);
  MQTTClient.publish(topic.c_str(),message.c_str());
}