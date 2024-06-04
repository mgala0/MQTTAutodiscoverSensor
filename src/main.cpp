/**
 * @file main.cpp
 * @author M.A.G (m.gala@mgala.eu)
 * @brief "Dummy sensor" example for HA MQTT autodiscover Arduino (ESP) sensor 
 * @version 0.1
 * @date 2024-06-03
 * 
 * @copyright Copyright (c) 2024
 * "Dummy sensor" example for HA MQTT autodiscover Arduino (ESP) sensor 
 * source informations from HA documentation and 
 * https://mpolinowski.github.io/docs/Automation_and_Robotics/Home_Automation/2022-07-10-home-assistant-mqtt-autodiscovery-part-i/2022-07-10#example--motion-detection-binary-sensor 
 * 
 */
#include <Arduino.h>
#include <WiFi.h>
#include "WiFiCred.h"     //defined ssid, passwords, user here. Should contain these definitions: #define SSID #define WIFI_PASSWD #define MQTT_USER #define MQTT_PASSWD 
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define BAUDRATE 115200                               //for debug serial port
#define MQTT_BROKER_IP "192.168.254.25"               
#define HOSTNAME "ESPLiveMiniDummySensor"             //hostname for ESP board, used also as sensor name
#define MQTT_CLIENT_NAME "ESP_LIVE_MINI"

float svalue = 20.0;                                  // fake sensor value to send

/**
 * @brief sensor properties and measured value structure
 * 
 */
struct sensor
{
  String device_class;
  String name;
  String state_topic;
  String unit_of_measurement;
  String value_template;
  String configuration_topic;
  float value;
};
typedef struct sensor AutodetectSensor;

/**
 * @brief Sensor definition and initialization 
 * One sensor has one measured value so if you have more sensor attached to one ESP board (or one with multiple measurements like BMP series) 
 * you should define one sensor for every measured value
 */
AutodetectSensor ESPDummySensor = {"temperature", "DummyTempSensor", "home/livingroom/" + String(HOSTNAME), "°C", "{{ value_json.temperature}}", "homeassistant/sensor/" + String(HOSTNAME) + "/config"};   

WiFiClient ESP32Client;
PubSubClient MQTTClient(ESP32Client);

void WiFiConnect(const char *ssid, const char *password);
void MQTTConnect(const char *server, const char *user, const char *passwd);
void MQTTSendMessage(String topic, String message);
void MQTTSendDiscoveryMsg(AutodetectSensor sensor);
void MQTTSendPayloadMessage(AutodetectSensor sensor);

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(BAUDRATE);
  WiFiConnect(SSID, WIFI_PASSWD);
  MQTTConnect(MQTT_BROKER_IP, MQTT_USER, MQTT_PASSWD); //connect to MQTT broker
  MQTTSendDiscoveryMsg(ESPDummySensor);                //send sensor discovery and configuration messafe to MQTT broker
}

void loop()
{
  //emulate sensor reading  
  svalue++; // change "measured value", add 1degC
  if (svalue > 35)
    svalue = 20; //"measured temperature" cannot be too high ;) 
  //above lines in real sensor should be replaced with procedure that read data from hardware like BMP, DHT series, etc.  
  ESPDummySensor.value = svalue; //assign "measured value" to sensor structure member  
  MQTTSendPayloadMessage(ESPDummySensor);
  Serial.println("KABOOM!!!"); //debug, "hello world, i'm alive" replacement XD"
  delay(2000);
}

/**
 * @brief Connect to wifi and set hostname
 * 
 * @param ssid 
 * @param password 
 */

void WiFiConnect(const char *ssid, const char *password)
{
  WiFi.setHostname(HOSTNAME); // define hostname FIRST
  WiFi.mode(WIFI_STA);        // then mode, this order is crucial to set hostname for ESP board
  WiFi.begin(ssid, password);
  Serial.println(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print(F("Connected to WiFi network with IP Address: "));
  Serial.println(WiFi.localIP());
  delay(1000);
}

/**
 * @brief  Connect to MQTT broker
 * 
 * @param server 
 * @param user 
 * @param passwd 
 */
void MQTTConnect(const char *server, const char *user, const char *passwd)
{
  MQTTClient.setServer(server, 1883);
  while (!MQTTClient.connected())
  {
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

/**
 * @brief General function to publish message to MQTT broker in given topic
 * 
 * @param topic 
 * @param message 
 */

void MQTTSendMessage(String topic, String message)
{
  if (!MQTTClient.connected())
    MQTTConnect(MQTT_BROKER_IP, MQTT_USER, MQTT_PASSWD);
  MQTTClient.publish(topic.c_str(), message.c_str());
}

/*
 * @brief Function to send discovery message to MQTT broker 
 * 
 * @param sensor Sensor to setup in HA/Domoticz 
 */
void MQTTSendDiscoveryMsg(AutodetectSensor sensor)
{
  String MQTT_message;
  JsonDocument MQTT_configuration_message;
  MQTT_configuration_message["device_class"] = sensor.device_class;
  MQTT_configuration_message["name"] = sensor.name;
  MQTT_configuration_message["state_topic"] = sensor.state_topic;
  MQTT_configuration_message["unit_of_measurement"] = sensor.unit_of_measurement;
  MQTT_configuration_message["value_template"] = sensor.value_template;
  serializeJson(MQTT_configuration_message, MQTT_message);
  MQTTSendMessage(sensor.configuration_topic, MQTT_message);
  Serial.println(MQTT_message); //debug
}

/**
 * @brief Function to send payload message to MQTT broker (send measured data) 
 * 
 * @param sensor sensor from which send measured value
 */
void MQTTSendPayloadMessage(AutodetectSensor sensor)
{
  String MQTT_message;
  JsonDocument MQTT_payload_message;
  MQTT_payload_message[sensor.device_class] = sensor.value;
  serializeJson(MQTT_payload_message, MQTT_message);
  MQTTSendMessage(sensor.state_topic, MQTT_message);
  Serial.println(MQTT_message); //debug
}