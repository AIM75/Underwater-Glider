#include "CommunicationModule.h"
// Configuration
const char* ssid = "AIM-MIA";
const char* password = "123456789";
IPAddress local_ip(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
const uint16_t wifi_port = 42050;

WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);

String data, cmd;

void setup() {
  Serial.begin(115200);
  wifiComms.begin();
}
void loop() {
  cmd = wifiComms.receiveCommand();
  if (cmd.equals("End"))
    wifiComms.end();
  Serial.println(cmd);
  data = "Hello\n";
  wifiComms.sendData(data);
}