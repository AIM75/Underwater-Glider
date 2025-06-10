#include "PitchController.h"

// Stepper motor pins (adjust for your ESP32 board)
#define STEP_PIN 33
#define DIR_PIN 25

// Mechanical parameters (adjust for your system)
const uint16_t STEPS_PER_REV = 200*32;    // 1.8° stepper
const float MAX_TRAVEL = 100.0f;       // mm
const float LEAD_SCREW_PITCH = 2.0f;   // mm/revolution

PitchController pitchController(STEP_PIN, DIR_PIN, STEPS_PER_REV, MAX_TRAVEL, LEAD_SCREW_PITCH);

void setup() {
    Serial.begin(115200);
    pitchController.begin();
    Serial.println("Pitch Controller Ready");
}

void loop() {
        float desiredPitch = 0+ millis()/1000;
        Serial.print("Setting pitch to: ");
        Serial.print(desiredPitch);
        Serial.println(" degrees");
        
        pitchController.setTargetPitch(desiredPitch);
    
    pitchController.update();
    
    // Optional: Print current position periodically
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        lastPrint = millis();
        Serial.print("Current position: ");
        Serial.print(pitchController.getCurrentPosition());
        Serial.println(" mm");
    }
}