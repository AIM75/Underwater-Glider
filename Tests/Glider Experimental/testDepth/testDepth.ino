#include <Wire.h>
#include "MPU6050dmp.h"
#include "Orientation.h"

// ESP32 I2C pins (default: SDA=21, SCL=22)
Orientation orient; // Update if using different pins

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial monitor
  
  Serial.println("Initializing MPU6050...");
  if (!orient.begin()) {
    Serial.println("MPU6050 initialization failed!");
    while (1);
  }
  Serial.println("MPU6050 ready!");
}

void loop() {
  if (orient.update()) {
    Serial.print("°\tPitch: ");
    Serial.print(orient.getPitch());
  }
  delay(100);
}