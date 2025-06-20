#include "CommunicationModule.h"
#include <ESP32Servo.h>
// Configuration
const char* ssid = "AIM-MIA";
const char* password = "123456789";
IPAddress local_ip(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
const uint16_t wifi_port = 42050;

const uint16_t stepsPerRevolution = 200*32;    // 1.8° stepper

WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);

String data, cmd;
Servo ser;

void setup() {
  Serial.begin(115200);
  pinMode(26, INPUT);
  pinMode(27, INPUT);
  pinMode(13, OUTPUT);
  wifiComms.begin();
  ser.attach(32);
}
void loop() {
  cmd = wifiComms.receiveCommand();
  if (cmd.equals("End"))
    wifiComms.end();
  Serial.println(cmd);
  data = "Hello\n";
  wifiComms.sendData(data);

  ser.write(160);
  for (int i = 0; i < stepsPerRevolution; i++) {
  delayMicroseconds(50);
  Serial.println(digitalRead(26));
  Serial.println(digitalRead(27));
  Serial.println("---------------------------------");
  }
  delay(1000); // Wait for 1 second before changing direction
  ser.write(30);

  for (int i = 0; i < stepsPerRevolution; i++) {
  delayMicroseconds(50);
  Serial.println(digitalRead(26));
  Serial.println(digitalRead(27));
  Serial.println("---------------------------------");
  }
  delay(1000); // Wait for 1 second before changing direction
}