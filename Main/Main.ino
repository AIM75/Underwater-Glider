#include "Includes.h"

// Constants
const float SURFACE_DEPTH = 0.5f;      // 0.5 meters considered surface
const unsigned long SURFACE_COMNS_TIME = 60000; // 1 minute surface comms
const float MAX_PITCH = 30.0f;         // Maximum pitch angle for gliding

// Module instances
MPX5010 depthSensor(34);
Orientation orientation;
SDLogger sdCard;
BallastServo ballast(25);
MPU6050DMP imu;

// Mission parameters
struct MissionParams {
    float divePitch;
    float ascendPitch;
    float targetDepth;
};

MissionParams currentMission = {
    .divePitch = 15.0f,    // Default dive pitch
    .ascendPitch = -15.0f,  // Default ascend pitch
    .targetDepth = 10.0f    // 10 meters target depth
};

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
    
    Serial.println("System ready");
}

bool initializeModules() {
    depthSensor.begin();
    if (!orientation.begin()) return false;
    if (!sdCard.begin()) return false;
    ballast.begin();
    
    // IMU initialization with retries
    for (int i = 0; i < 5; i++) {
        if (imu.begin()) return true;
        delay(1000);
    }
    return false;
}

void loop() {
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
    
    if (orientation.update()) {
        float pitch = orientation.getPitch();
        float roll = orientation.getRoll();
        int8_t ballastPos = ballast.getPosition();
        
        // Create data string
        String data = String(millis()) + "," +
                     String(depth, 2) + "," +
                     String(pitch, 1) + "," +
                     String(roll, 1) + "," +
                     String(ballastPos);
        
        // Log data when submerged
        if (!isAtSurface) {
            sdCard.logData(data);
        }
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
    enableCommunications();
    uploadData();
    checkForCommands();
}

void runStateMachine() {
    switch (currentState) {
        case SURFACE_COMNS:
            handleSurfaceComms();
            break;
        case DESCENDING:
            handleDescent();
            break;
        case GLIDING_DOWN:
            handleGlideDown();
            break;
        case GLIDING_UP:
            handleGlideUp();
            break;
        case ASCENDING:
            handleAscent();
            break;
        case EMERGENCY:
            handleEmergency();
            break;
    }
}

// State handlers
void handleSurfaceComms() {
    ballast.setPosition(0); // Neutral buoyancy
    
    // Timeout or receive dive command
    if (millis() - lastSurfaceTime > SURFACE_COMNS_TIME) {
        currentState = DESCENDING;
        disableCommunications();
    }
}

void handleDescent() {
    ballast.setPosition(-80); // Negative buoyancy
    
    // Switch to gliding when descending
    if (orientation.getPitch() > currentMission.divePitch - 5.0f) {
        currentState = GLIDING_DOWN;
    }
}

void handleGlideDown() {
    // Maintain pitch angle for gliding descent
    adjustPitch(currentMission.divePitch);
    
    // Check depth for transition
    if (depthSensor.readDepthCm() / 100.0f >= currentMission.targetDepth) {
        currentState = ASCENDING;
    }
}

void handleGlideUp() {
    // Maintain pitch angle for gliding ascent
    adjustPitch(currentMission.ascendPitch);
    
    // Check for surface approach
    if (depthSensor.readDepthCm() / 100.0f < currentMission.targetDepth / 2) {
        currentState = ASCENDING;
    }
}

void handleAscent() {
    ballast.setPosition(80); // Positive buoyancy
    
    // Switch to gliding when ascending
    if (orientation.getPitch() < currentMission.ascendPitch + 5.0f) {
        currentState = GLIDING_UP;
    }
}

void handleEmergency() {
    ballast.setPosition(100); // Full positive buoyancy
    adjustPitch(0);           // Level orientation
}

// Helper functions
void adjustPitch(float targetPitch) {
    float currentPitch = orientation.getPitch();
    float error = targetPitch - currentPitch;
    
    // Simple proportional control (adjust gain as needed)
    int8_t adjustment = constrain(error * 2, -20, 20);
    ballast.setPosition(adjustment);
}

void uploadData() {
    String data;
    while ((data = sdCard.readNextLine()).length() > 0) {
        // Send to LabVIEW
        Serial.println("SEND:" + data);
        // Implement actual WiFi transmission here
    }
}

void checkForCommands() {
    // Example command format: "PITCH:15:-10" (dive:ascend angles)
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd.startsWith("PITCH:")) {
            int colon1 = cmd.indexOf(':');
            int colon2 = cmd.indexOf(':', colon1 + 1);
            
            if (colon1 != -1 && colon2 != -1) {
                float dive = cmd.substring(colon1 + 1, colon2).toFloat();
                float ascend = cmd.substring(colon2 + 1).toFloat();
                
                // Constrain pitch angles
                dive = constrain(dive, 5.0f, MAX_PITCH);
                ascend = constrain(ascend, -MAX_PITCH, -5.0f);
                
                currentMission.divePitch = dive;
                currentMission.ascendPitch = ascend;
                
                Serial.print("New pitch angles - Dive:");
                Serial.print(dive);
                Serial.print(" Ascend:");
                Serial.println(ascend);
            }
        }
    }
}

void enableCommunications() {
    Serial.println("COMMS:ON");
}

void disableCommunications() {
    Serial.println("COMMS:OFF");
}

void emergencyProcedure() {
    currentState = EMERGENCY;
    while (true) {
        handleEmergency();
        delay(100);
    }
}
