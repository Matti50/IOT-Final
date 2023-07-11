#include "TestGroundHumidityReader.h"
#include <Arduino.h>

TestGroundHumidityReader::TestGroundHumidityReader()
{
}

TestGroundHumidityReader::~TestGroundHumidityReader()
{
}

int TestGroundHumidityReader::ReadHumidity()
{
    return random(10, 80);
}
