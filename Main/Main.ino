#include "Application.h"

void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  // Initialize modules
  digitalWrite(2,HIGH);
  initializeModules();
}

void loop() {
  updateSensorData();
  runStateMachine();
  delay(100);
}

