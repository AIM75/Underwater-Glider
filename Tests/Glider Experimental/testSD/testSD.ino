#include "sdCardModule.h"

// ESP32 SD Card pins (default HSPI)
SDLogger sdCard; // CS on GPIO5

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  if (!sdCard.begin()) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("SD Card ready!");
}

void loop() {
  String data = "ESP32 Test: " + String(millis());
  if (sdCard.logData(data)) {
    Serial.println("Logged: " + data);
  } else {
    Serial.println("Log failed!");
  }
  delay(2000);
}