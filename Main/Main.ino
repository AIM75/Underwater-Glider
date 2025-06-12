#include "Application.h"

void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  // Initialize modules
  if (!initializeModules()) {
    emergencyProcedure();
    digitalWrite(2,LOW);
  } else {
    Serial.println("System ready");
    digitalWrite(2,HIGH);
  }
}

void loop() {
  updateSensorData();
  runStateMachine();
  delay(100);
}

