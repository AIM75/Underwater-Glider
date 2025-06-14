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
  delay(100);
}
