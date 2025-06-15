#include "CommunicationModule.h"

WiFiComms::WiFiComms(const char* ssid, const char* password,
                     IPAddress local_ip, IPAddress gateway, IPAddress subnet,
                     uint16_t port)
  : _ssid(ssid), _password(password),
    _local_ip(local_ip), _gateway(gateway), _subnet(subnet),
    _port(port), _server(port) {}

void WiFiComms::begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(_local_ip, _gateway, _subnet);
  WiFi.softAP(_ssid, _password);
  _server.begin();
}

void WiFiComms::end() {
  if (_client) _client.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
}

bool WiFiComms::isConnected() {
  if (!_client || !_client.connected()) {
    _client = _server.available();
  }
  return _client && _client.connected();
}

String WiFiComms::receiveCommand() {

  if (isConnected() && _client.available()) {
    String cmd = _client.readStringUntil(';');
    cmd.trim();
    return cmd;
  }
  return "";
}

void WiFiComms::sendData(const String& data) {
  if (isConnected()) {
    _client.println(data);
  }
}