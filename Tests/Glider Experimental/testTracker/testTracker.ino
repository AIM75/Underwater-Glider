#include "PositionTracker.h"

Orientation orient;
MPX5010 depthSensor(GPIO_NUM_34);
PositionTracker tracker(orient, depthSensor);

void setup() {
    Serial.begin(115200);
    
    orient.begin();
    depthSensor.begin();
    tracker.begin(5); // Medium priority task
    
    // Calibrate on stable surface
    Serial.println("Perform accelerometer calibration...");
    tracker.calibrateAccelerometer(2000);
    Serial.println("Calibration complete!");
}

void loop() {
    // Main thread can focus on other tasks
    Serial.printf("Position: X=%.2fm Z=%.2fm\n", 
                 tracker.getX(),depthSensor.readDepthCm()/100.0);
    
    delay(100); // Update display at 10Hz
}