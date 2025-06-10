#include "PitchController.h"

// Pin configuration for ESP32 DOIT DevKit v1
#define STEP_PIN 26   // GPIO26 connected to stepper driver STEP
#define DIR_PIN 27    // GPIO25 connected to stepper driver DIR
#define SLEEP_PIN 33  // GPIO33 connected to stepper driver SLEEP

// Glider configuration
PitchConfig config = {
  // Pin configuration
  .step_pin = STEP_PIN,
  .dir_pin = DIR_PIN,
  .sleep_pin = SLEEP_PIN,

  // Mechanical properties
  .steps_per_rev = 200,
  .lead_screw_pitch = 8.0f,  // 8mm lead screw
  .max_travel = 20.0f,       // 100mm max travel

  // Physics parameters (mm from tail)
  .cob_position = 205.55f,   // Center of buoyancy
  .cop_position = 199.5f,    // Center of pressure
  .mechanism_start = 50.0f,  // Mass mechanism start position

  // Driver configuration
  .microsteps = 32  // 1/32 microstepping
};

PitchController pitchController(config);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection
  
  pitchController.begin();
  
  Serial.println(F("Underwater Glider Pitch Controller"));
  Serial.println(F("System initialized"));
  printStatus();
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }

  // Update controller state
  pitchController.update();
  
  // Periodic status report
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    printStatus();
  }
}

void handleSerialCommand() {
  char cmd = Serial.read();
  float angle;
  int phase;
  
  switch (cmd) {
    case 'a': // Set pitch angle
      while(!(angle = Serial.parseFloat()));
      pitchController.setTargetPitch(angle);
      Serial.print(F("Set target pitch to: "));
      Serial.print(angle);
      Serial.println(F("°"));
      break;
      
    case 'u': // Set dive phase 1=descend, else=ascend)
      while(!(phase = Serial.parseFloat()));
      if (phase == 1) {
        pitchController.setDivePhase(PitchController::DivePhase::DESCENDING);
        Serial.println(F("Phase set to DESCENDING"));
      } else {
        pitchController.setDivePhase(PitchController::DivePhase::ASCENDING);
        Serial.println(F("Phase set to ASCENDING"));
      }
      break;
      
    case 's': // Emergency stop
      pitchController.emergencyStop();
      Serial.println(F("EMERGENCY STOP ACTIVATED"));
      break;
      
    case 'p': // Print status
      printStatus();
      break;
      
    default:
      Serial.println(F("Unknown command"));
      break;
  }
  
  // Clear serial buffer
  while (Serial.available()) Serial.read();
}

void printStatus() {
  Serial.println(F("\n=== Glider Status ==="));
  Serial.print(F("Dive Phase: "));
  Serial.println(pitchController.getCurrentPhase() == PitchController::DivePhase::ASCENDING ? "ASCENDING" : "DESCENDING");
  
  Serial.print(F("Current Pitch: "));
  Serial.print(pitchController.getCurrentPitch(), 1);
  Serial.println(F("°"));
  
  Serial.print(F("Mass Position: "));
  Serial.print(pitchController.getCurrentMassPosition(), 1);
  Serial.println(F("mm"));
  
  Serial.print(F("Motor State: "));
  Serial.println(pitchController.isMoving() ? "MOVING" : "IDLE");
  Serial.println();
}