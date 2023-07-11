#include "TestWateringSystem.h"
#include <Arduino.h>

TestWateringSystem::TestWateringSystem(int pin)
{
    waterLevel = 90;
    outputPin = pin;
    pinMode(pin, OUTPUT);
    lastTimeWaterStarted = 0;
    outputPinStatus = false;
}

TestWateringSystem::~TestWateringSystem()
{
}

bool TestWateringSystem::IsWaterLevelSufficient(){
    return waterLevel >= 50;
}

bool TestWateringSystem::IsCurrentlyWatering(){
    return outputPinStatus;
}

void TestWateringSystem::Water(){
    Serial.println("regando...");
    digitalWrite(outputPin, HIGH);
    lastTimeWaterStarted = millis();
    waterLevel = waterLevel - 40;
    outputPinStatus = true;
}

void TestWateringSystem::TryToStopWater(){
   if (outputPinStatus){
     digitalWrite(outputPin, LOW);
     outputPinStatus = false;
   }
}
