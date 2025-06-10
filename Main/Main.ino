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

  /*// Debug output
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print("State:");
    Serial.print(currentState);
    Serial.print(" | TargetPitch:");
    Serial.print(targetPitch);
    Serial.print("° | CurrentPitch:");
    Serial.print(pitch);
    Serial.print("° | TargetDepth:");
    Serial.print(diveDepth);
    Serial.print("m | CurrentDepth:");
    Serial.print(currentDepth);
    Serial.println("m");
  }
  */
  delay(1000);
}

