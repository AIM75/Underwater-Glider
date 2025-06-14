#include <Arduino.h>
#include "BallastServo.h"

// Create a servo object on GPIO pin 13
BallastServo myServo(32);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection on boards with native USB
  
  Serial.println("Ballast Servo Example");
  
  // Initialize the servo at neutral position (0)
  myServo.begin(0);
  
  delay(1000); // Give the servo time to reach initial position
}

void loop() {
  Serial.println("Moving to -100 (full descent position)");
  myServo.setPosition(180);
  Serial.print("Current position: ");
  Serial.println(myServo.getPosition());
  delay(2000);
  
  Serial.println("Moving to 0 (neutral position)");
  myServo.setPosition(0);
  Serial.print("Current position: ");
  Serial.println(myServo.getPosition());
  delay(2000);
  
  Serial.println("Moving to 100 (full ascent position)");
  myServo.setPosition(100);
  Serial.print("Current position: ");
  Serial.println(myServo.getPosition());
  delay(2000);
  
  // Demonstrate intermediate positions
  for (int pos = 180; pos >= 0 ; pos -= 20) {
    Serial.print("Moving to position: ");
    Serial.println(pos);
    myServo.setPosition(pos);
    delay(1000);
  }
  
  delay(2000); // Pause before repeating
}