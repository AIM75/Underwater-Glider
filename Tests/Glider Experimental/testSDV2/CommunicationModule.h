#ifndef COMM_MOD_H
#define COMM_MOD_H

#include <WiFi.h>
#include <SD.h> // Include SD library

class WiFiComms {
public:
    WiFiComms(const char* ssid, const char* password, 
             IPAddress local_ip, IPAddress gateway, IPAddress subnet,
             uint16_t port);

    void begin();
    void end();
    bool isConnected();
    String receiveCommand();
    void sendData(const String& data);

private:
    const char* _ssid;
    const char* _password;
    IPAddress _local_ip;
    IPAddress _gateway;
    IPAddress _subnet;
    uint16_t _port;
    
    WiFiServer _server;
    WiFiClient _client;
};



#endif