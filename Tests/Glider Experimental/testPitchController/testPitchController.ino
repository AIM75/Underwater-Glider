#include "PitchController.h"
#define _stepPIN 26    // stepper driver STEP pin
#define _dirPIN 33     // stepper driver DIR pin
#define _slpPIN 27     // stepper driver DIR pin
#define _limSW1PIN 14  // Limit switch 1 pin
#define _limSW2PIN 12  // Limit switch 2 pin

// Update these pin numbers to match your actual wiring
PitchConfig config = {
  .step_pin = _stepPIN,
  .dir_pin = _dirPIN,
  .sleep_pin = _slpPIN,
  .limit_switch_min_pin = _limSW1PIN,
  .limit_switch_max_pin = _limSW2PIN,
  // Keep other defaults or adjust as needed
};

PitchController pitchController(config);

void setup() {
  Serial.begin(115200);
  pitchController.begin();
  Serial.println("Starting automatic stepper test...");
}

void loop() {
  // Run calibration (homes to min position)
  Serial.println("Running calibration (homing to min position)...");
  pitchController.calibrate();
  delay(1000);

  // Move to max position (using pitch command)
  Serial.println("Moving to max position...");
  pitchController.setTargetPitch(10); // Use a pitch that should go to max
  while(pitchController.getStepsToGo() != 0) {
    pitchController.update();
  }
  delay(1000);

  // Move back to min position (using pitch command)
  Serial.println("Moving back to min position...");
  pitchController.setTargetPitch(-10); // Use a pitch that should go to min
  while(pitchController.getStepsToGo() != 0) {
    pitchController.update();
  }
  delay(1000);

  // Move to center position
  Serial.println("Moving to center position...");
  pitchController.setTargetPitch(0); // Should move to neutral position
  while(pitchController.getStepsToGo() != 0) {
    pitchController.update();
  }
  delay(1000);

  // Print current status
  Serial.println("Current status:");
  Serial.print("Mass position: ");
  Serial.print(pitchController.getCurrentMassPosition());
  Serial.println(" mm");
  Serial.print("Pitch angle: ");
  Serial.print(pitchController.getCurrentPitch());
  Serial.println(" deg");
  
  Serial.println("Test cycle complete. Restarting in 5 seconds...");
  delay(5000);
}