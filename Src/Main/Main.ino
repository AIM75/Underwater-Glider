#include "Application.h"

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // JUST ESP32 Indication
  digitalWrite(2, HIGH);

  // Waiting For Initialization
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
  delay(1000);
}
