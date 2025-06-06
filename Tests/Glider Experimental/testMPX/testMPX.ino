#include "DepthModule.h"

bool isAtSurface = false;

// Module instances
MPX5010 depthSensor(13);
String command = "0";


void setup() {
    Serial.begin(115200);


}


void loop() {
    // Update sensor readings
    updateSensorData();

    
    delay(100); // Main loop delay
}

void updateSensorData() {
    float depth = depthSensor.readDepthCm() / 100.0f; // meters

        
    // Create data string
    String data = String(millis()) + "," +
                  String(depth, 2);
    Serial.print("analogRead:  ");
    Serial.println(analogRead(13));
    Serial.print("Depth:  ");              
    Serial.println(depth);
    Serial.print("kPa:  ");
    Serial.println(depthSensor.readPressurekPa());
    Serial.print("CmH2O:  ");
    Serial.println(depthSensor.readPressureCmH2O());
    Serial.println("-----------------------------------------------------------------");
    delay(400);
}


