#ifndef TemperatureAndHumidityReader_H
#define TemperatureAndHumidityReader_H
#include "DHT.h"

class TemperatureAndHumidityReader
{
private:
    DHT *dht;
public:
    TemperatureAndHumidityReader(int dhtPin);
    ~TemperatureAndHumidityReader();
    int ReadHumidity();
    int ReadTemperature();
};

#endif
