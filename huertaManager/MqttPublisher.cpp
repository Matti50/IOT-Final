#include "MqttPublisher.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

void callback(char* topic, byte* message, unsigned int length)
{
  Serial.println("callback");
}

MqttPublisher::MqttPublisher(char* hostName, int portNumber, WiFiClient wclient)
{
  host = hostName;
  port = portNumber;
  client_name = "ESP32Client_1";

  client = wclient;

  mqttClient = new PubSubClient(client);

  mqttClient->setServer(host, port);
  mqttClient->setCallback(callback);
}

MqttPublisher::~MqttPublisher()
{
}

void MqttPublisher::Publish(char* topic, char* payload)
{
    mqttClient->publish(topic, payload);
}

void MqttPublisher::Connect()
{
    while (!mqttClient->connected()) {
    Serial.print(" MQTT connection...");
    
    if (mqttClient->connect("bli12", "", "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
