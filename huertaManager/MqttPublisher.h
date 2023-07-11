#ifndef MqttPublisher_H
#define MqttPublisher_H

#include <PubSubClient.h>

#include <WiFi.h>

class MqttPublisher
{
private:
    char* host;
    int port;
    char* client_name;
    char* user;
    char* password;
    PubSubClient *mqttClient;
    WiFiClient client;
public:
    MqttPublisher(char* hostName, int portNumber, WiFiClient client);
    ~MqttPublisher();
    void Publish(char* topic, char* payload);
    void Connect();
};

#endif
