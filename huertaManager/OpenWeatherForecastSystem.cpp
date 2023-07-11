#include "OpenWeatherForecastSystem.h"
#include <Arduino.h>

OpenWeatherForecastSystem::OpenWeatherForecastSystem()
{
}

OpenWeatherForecastSystem::~OpenWeatherForecastSystem()
{
}

int OpenWeatherForecastSystem::GetRainProbability(){
    return random(10, 100);
}