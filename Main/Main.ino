#include "Includes.h"

// Constants
const float SURFACE_DEPTH = 0.1f;      // 0.1 meters considered surface
const unsigned long SURFACE_COMNS_TIME = 60000; // 1 minute surface comms
const float MAX_PITCH = 30.0f;         // Maximum pitch angle for gliding
const float MAX_DEPTH = 0.5f;

// SSID and password of Wifi connection:
const char* ssid = "AIM-MIA";
const char* password = "123456789";

IPAddress local_IP(192,168,1,22);
IPAddress gateway(192,168,1,5);
IPAddress subnet(255,255,255,0);
WiFiServer underwater_server(42050);
WiFiClient client = underwater_server.available();

// Module instances
MPX5010 depthSensor(13);
Orientation orientation;
SDLogger sdCard;
BallastServo ballast(25);

String command = "0";

// State machine
enum GliderState {
    SURFACE_COMNS,
    DESCENDING,
    GLIDING_DOWN,
    GLIDING_UP,
    ASCENDING,
    EMERGENCY
};

GliderState currentState = SURFACE_COMNS;
unsigned long lastSurfaceTime = 0;
bool isAtSurface = false;

void setup() {
    Serial.begin(115200);
    
    // Initialize modules
    if (!initializeModules()) {
        emergencyProcedure();
    }

    // Set up WiFi in AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  underwater_server.begin();
  Serial.println("Server started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
   
    Serial.println("System ready");
}

bool initializeModules() {
    depthSensor.begin();
    if (!orientation.begin()) return false;
    if (!sdCard.begin()) return false;
    ballast.begin();
    return false;
}

void loop() {
   client = underwater_server.available();
    // Update sensor readings
    updateSensorData();
    // Check surface status
    checkSurface();
    
    // State machine execution
    runStateMachine();
    
    delay(100); // Main loop delay
}

void updateSensorData() {
    float depth = depthSensor.readDepthCm() / 100.0f; // meters
    
    float pitch = orientation.getPitch();
    float roll = orientation.getRoll();
    int8_t ballastPos = ballast.getPosition();
        
    // Create data string
    String data = String(millis()) + "," +
                  String(depth, 2) + "," +
                  String(pitch, 1) + "," +
                  String(roll, 1) + "," +
                  String(ballastPos);
    Serial.println(analogRead(13));
    // Log data when submerged
    if (!isAtSurface) {
      sdCard.logData(data);
    }
}

void checkSurface() {
    float depth = depthSensor.readDepthCm() / 100.0f;
    
    if (depth < SURFACE_DEPTH) {
        if (!isAtSurface) {
            onSurface();
        }
        isAtSurface = true;
    } else {
        isAtSurface = false;
    }
}

void onSurface() {
    lastSurfaceTime = millis();
    currentState = SURFACE_COMNS;
    uploadData();
    checkForCommands();
}


void uploadData() {
    String data;
    while ((data = sdCard.readNextLine()).length() > 0) {
        // Send to LabVIEW
        while (client) {
    
            Serial.println("New Client connected");
            float readTime = millis();
            while (client.connected()) {
                // Send the data to client
                client.println(data);
                if ((millis()-readTime) > 5000){                                //strcmp(client.read(),"X") == 0
                    client.println("Client not connected");
                    break;}
            }
        if ((millis()-readTime) > 5000){                                //strcmp(client.read(),"X") == 0
            client.println("Client not connected");
            break;}
        }
    }
}    

void checkForCommands() {
    // Example command format: "PITCH:15:-10" (dive:ascend angles)
    if (client.available()) {
        command = client.read();
    }
    else {
        command = "Still";
    }
}

void runStateMachine() {

}

void enableCommunications() {
    Serial.println("COMMS:ON");
}

void disableCommunications() {
    Serial.println("COMMS:OFF");
}

void emergencyProcedure() {
    currentState = EMERGENCY;
/*    while (true) {
        delay(100);
    }*/
}
