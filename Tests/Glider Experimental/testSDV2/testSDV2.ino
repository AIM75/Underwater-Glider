#include "CommunicationModule.h"
#include "DataLogger.h"

// WiFi configuration
const char* ssid = "AIM-MIA";
const char* password = "123456789";
IPAddress local_ip(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
const uint16_t wifi_port = 42050;

WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);
DataLogger dataLogger;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize data logger
    if (!dataLogger.begin()) {
        Serial.println("DataLogger initialization failed!");
    } else {
        Serial.println("DataLogger initialized successfully");
    }

    // Initialize WiFi
    wifiComms.begin();
    
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Send commands to ESP32:");
    Serial.println("- 'GET_DATA' to retrieve all logged data");
    Serial.println("- 'CLEAR_DATA' to clear the log");
    Serial.println("- 'SAMPLE_DATA' to generate 10 random samples");
}

void loop() {
    // Handle incoming commands
    String command = wifiComms.receiveCommand();
    if (command.length() > 0) {
        Serial.println("Received command: " + command);
        
        if (command == "GET_DATA") {
            String loggedData = dataLogger.readData();
            if (loggedData.length() > 0) {
                wifiComms.sendData("=== BEGIN DATA ===");
                wifiComms.sendData(loggedData);
                wifiComms.sendData("=== END DATA ===");
                Serial.println("Sent all logged data to client");
            } else {
                wifiComms.sendData("No data available");
                Serial.println("No data to send");
            }
        } 
        else if (command == "CLEAR_DATA") {
            if (dataLogger.clearData()) {
                wifiComms.sendData("Data cleared successfully");
                Serial.println("Data cleared");
            } else {
                wifiComms.sendData("Failed to clear data");
                Serial.println("Failed to clear data");
            }
        }
        else if (command == "SAMPLE_DATA") {
            generateSampleData(10); // Generate 10 random samples
            wifiComms.sendData("Generated 10 random samples");
        }
        else {
            wifiComms.sendData("Unknown command: " + command);
        }
    }
    
    delay(100); // Small delay to prevent busy waiting
}

void generateSampleData(int count) {
    Serial.println("Generating " + String(count) + " random samples...");
    
    for (int i = 0; i < count; i++) {
        // Generate random sensor data
        unsigned long timestamp = millis() + (i * 1000);
        float temperature = 20.0 + (float)random(0, 150)/10.0; // 20.0-35.0°C
        float humidity = 40.0 + (float)random(0, 400)/10.0;   // 40.0-80.0%
        int light = random(0, 1000);                           // 0-1000 lux
        
        // Format data as CSV
        String dataEntry = String(timestamp) + "," + 
                         String(temperature) + "," + 
                         String(humidity) + "," +
                         String(light);
        
        // Log the data
        if (dataLogger.logData(dataEntry)) {
            Serial.println("Logged: " + dataEntry);
        } else {
            Serial.println("Failed to log: " + dataEntry);
        }
    }
}