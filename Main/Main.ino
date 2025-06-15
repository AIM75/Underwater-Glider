#include "Application.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  // Initialize modules
  digitalWrite(2, HIGH);
  wifiComms.begin();
  while (!wifiComms.isConnected()) {
    wifiComms.begin();
    while (wifiComms.isConnected()) {
      String cmd = wifiComms.receiveCommand();
      if (cmd.length() > 0 && cmd.startsWith("INIT_SEQ")) {
        initializeModules();
        wifiComms.sendData("INIT");
        break;
      }
    } 
  }
}

void loop() {
  DataLogging();
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
