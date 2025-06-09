#include "Application.h"

void setup() {
  Serial.begin(115200);

  // Initialize modules
  if (!initializeModules()) {
    emergencyProcedure();
  } else {
    Serial.println("System ready");
  }
}

void loop() {
  updateSensorData();
  runStateMachine();
  delay(100);
}

