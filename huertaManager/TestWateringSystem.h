#ifndef TestWateringSystem_H
#define TestWateringSystem_H

class TestWateringSystem
{
private:
    int waterLevel;
    long lastTimeWaterStarted; 
    int outputPin;
    bool outputPinStatus;
public:
    TestWateringSystem(int pin);
    ~TestWateringSystem();
    bool IsWaterLevelSufficient();
    void Water();
    bool IsCurrentlyWatering();
    void TryToStopWater();
};
#endif
