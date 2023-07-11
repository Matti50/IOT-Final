#include "TemperatureAndHumidityReader.h"
#include <Arduino.h>
#include "DHT.h"

#define DHTTYPE DHT11

TemperatureAndHumidityReader::TemperatureAndHumidityReader(int dhtPin)
{
  dht = new DHT(dhtPin, DHTTYPE);
  dht->begin();
}

TemperatureAndHumidityReader::~TemperatureAndHumidityReader()
{
}

int TemperatureAndHumidityReader::ReadHumidity()
{
  return dht->readHumidity();
}

int TemperatureAndHumidityReader::ReadTemperature()
{
  return dht->readTemperature();
}
