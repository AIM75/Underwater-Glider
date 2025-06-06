#include "BallastServo.h"

// ESP32 servo pin (can use any PWM-capable GPIO)
BallastServo ballast(34, 1000, 2000); 
Servo my;

void setup() {
  Serial.begin(115200);
  ballast.begin();
  Serial.println("Ballast Servo Test - ESP32");
}

void loop() {
  // Test full range
  for (int pos = 0; pos <= 90; pos += 10) {
    my.write(pos);
    Serial.print("Position: ");
    Serial.print(pos);
    Serial.println("%");
    delay(500);
  }
}