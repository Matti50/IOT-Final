#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#include "TestGroundHumidityReader.h"

const int DHT_PIN = 27;
#include "TemperatureAndHumidityReader.h"

#include "TestWateringSystem.h"
const int LED_PIN = 2; // replace with bomb pin

#define BOTtoken "BOT TOKEN" 
#define CHAT_ID "CHAT ID"

const char* ssid     = "SSID"; //cargar datos de la red wifi propia
const char* password = "PASSWORD"; //cargar datos de la red wifi propia

String deviceId = "";

#include "MqttPublisher.h"

bool wasDeviceSet = false;
const char* plant = "";
int preferredTimeToWater = -1;
int minimumAllowedHumidity = 20;
int maximumAllowedHumidity = 70;

WiFiUDP udp;
Coap coap(udp);

IPAddress dev_ip(192,168,100,6);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

TestGroundHumidityReader *groundHumidityReader;
TemperatureAndHumidityReader *temperatureAndHumidityReader;
TestWateringSystem *wateringSystem;
MqttPublisher *mqttPublisher;

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  Serial.println(packet.payloadlen);
  DynamicJsonDocument doc(packet.payloadlen);
  memcpy(p, packet.payload, packet.payloadlen);

  Serial.print("Received: ");
  Serial.println(p);
  Serial.println();
  
  DeserializationError err = deserializeJson(doc, p);
  if (err){
    Serial.println(err.f_str());  
    return;
  }

  int code = doc["code"];

  if (code == 200){
      plant = doc["name"];
      preferredTimeToWater = doc["time"];  
      Serial.print("Plant: ");
      Serial.println(plant);
      Serial.print("Preferred time: ");
      Serial.println(preferredTimeToWater);
      wasDeviceSet = true;
  }
  p[packet.payloadlen] = NULL;
}

void setup() {
  Serial.begin(9600);
  // connect to wifi and get IP address
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  groundHumidityReader = new TestGroundHumidityReader();
  temperatureAndHumidityReader = new TemperatureAndHumidityReader(DHT_PIN);
  wateringSystem = new TestWateringSystem(LED_PIN);

  WiFiClient client;
  mqttPublisher = new MqttPublisher("HOST ADDRESS", 1883, client);

  mqttPublisher->Connect();

  coap.response(callback_response);

  coap.start();

  bool messageSent = false;
  
  while (!wasDeviceSet){

    deviceId = WiFi.macAddress();
    int macAddrLen = deviceId.length() + 1;
  
    char macAddrArray[macAddrLen];
    
    deviceId.toCharArray(macAddrArray, macAddrLen);
    
    coap.put(dev_ip, 5683, "devices", macAddrArray);
  
    delay(500);
    coap.loop();
    Serial.println("passed");
    if (!wasDeviceSet){

        if (!messageSent){
          Serial.println("Enviando mensaje al bot");
          String message = "El dispositivo ";
          message += deviceId;
          message += " no esta configurado, por favor introduce una opcion: \n";
          message += "1. lechuga\n";
          message += "2. otro\n";
          bot.sendMessage(CHAT_ID, message, "");
          messageSent = true;
        }

        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while(!numNewMessages) {
          Serial.println("Esperando mensaje del usuario");
          delay(200);
          numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }

        handleNewMessages(numNewMessages, macAddrArray);
    }
  }
}

void loop() {
  int groundHumidity = groundHumidityReader->ReadHumidity();
  int temperature = temperatureAndHumidityReader->ReadTemperature();
  int humidity = temperatureAndHumidityReader->ReadHumidity();

  Serial.print("Ground humidity: ");
  Serial.println(groundHumidity);
  Serial.println(humidity);
  Serial.println(temperature);

  String payload;
  DynamicJsonDocument doc(200);
  doc["groundHumidity"] = groundHumidity;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  serializeJson(doc, payload);

  String topic = "device/";
  topic += deviceId;
  topic += "/environment";

  Serial.println(payload);
  Serial.println(topic);

  
  int charArrayLen = topic.length() + 1;
  char topicCharArray[charArrayLen];
  topic.toCharArray(topicCharArray, charArrayLen);

/*
char* output;
for(int x = 0; x < index-1; x++){
    size_t nullTerm = strlen(topicCharArray[x]); // Get current length
    output = topicCharArray[x];            // Start of string
    for (size_t i = 0; i < nullTerm; ++i){
        if(topicCharArray[x][i] != ':') {        // If it's NOT a colon
            *output++ = topicCharArray[x][i];    // Copy character 'up'
        } // if
                                          // If it WAS a colon, then
                                          // output does NOT get incremented
    } // for
    *output = '\0'; // End string off with NUL
    Serial.println(output);
} // for*/

  int payloadLen = payload.length() + 1;
  char payloadCharArray[payloadLen];
  payload.toCharArray(payloadCharArray, payloadLen);

  Serial.print("Topic to post: ");
  Serial.println(topicCharArray);

  Serial.print("Payload: ");
  Serial.println(payloadCharArray);
  
  mqttPublisher->Publish(topicCharArray, payloadCharArray);

  if (groundHumidity < minimumAllowedHumidity && !wateringSystem->IsCurrentlyWatering()){
      if (!wateringSystem->IsWaterLevelSufficient()){
        //inform in telegram that water needs to be refilled
        return;
      }

      wateringSystem->Water();
      
      //inform, send telegram notification
  }

  if (groundHumidity >= maximumAllowedHumidity){
      wateringSystem->TryToStopWater();
  }

  delay(2000);
}

void handleNewMessages(int numNewMessages, char* deviceId) {
  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);

    if (chat_id != CHAT_ID){
      bot.sendMessage(CHAT_ID, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    if (text == "/1") {
      String message = "{\"device\": \"";
      message += String(deviceId);
      message += "\", \"plant\": 1}";

      int messageLen = message.length() + 1;
      char messageArray[messageLen];
      message.toCharArray(messageArray, messageLen);
      
      coap.send(dev_ip, 5683, "devices",COAP_CON, COAP_POST, NULL, 0, (uint8_t *)messageArray, strlen(messageArray));
      delay(100);
      coap.loop();
    }
    else{
      bot.sendMessage(chat_id, "Por favor ingrese un valor correcto");
    }

  }

}
