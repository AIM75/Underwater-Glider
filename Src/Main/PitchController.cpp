#include "PitchController.h"
#include <Arduino.h>
#include <math.h>

PitchController::PitchController(const PitchConfig& config)
  : _config(config),
    _stepper(AccelStepper::DRIVER, config.step_pin, config.dir_pin),
    _current_phase(DivePhase::DESCENDING),
    _current_mass_pos(config.cob_position - _config.mechanism_start + 10.0f),  // Start 10mm ahead of COB tilting abit for more in the water
    _limit_min_hit(false),
    _limit_max_hit(false) {
}

void PitchController::begin() {
  pinMode(_config.sleep_pin, OUTPUT);
  pinMode(_config.limit_switch_min_pin, INPUT_PULLUP);
  pinMode(_config.limit_switch_max_pin, INPUT_PULLUP);

  _enableMotor();

  // Configure motion profile (all in mm/s)
  const float max_speed_mmps = 90.0f;    // Maximum speed (mm/s)
  const float max_accel_mmps2 = 100.0f;  // Acceleration (mm/s²)

  _stepper.setMaxSpeed(_mmToSteps(max_speed_mmps));
  _stepper.setAcceleration(_mmToSteps(max_accel_mmps2));

  // Calibrate on startup
  calibrate();
}
void PitchController::_hitLIMSW(int speed, float pos) {
  _stepper.setSpeed((speed / abs(speed)) * _mmToSteps(abs(speed)));
  _stepper.moveTo(_mmToSteps(pos));
  while (_stepper.distanceToGo() != 0) {
    _stepper.runSpeed();
  }
}

void PitchController::calibrate() {
  _enableMotor();

  // Move to min limit switch
  _stepper.setSpeed(-_mmToSteps(10.0f));  // Slow speed for homing
  while (digitalRead(_config.limit_switch_min_pin)) {
    _stepper.runSpeed();
  }
  _stepper.stop();
  _stepper.setCurrentPosition(0);
  _current_mass_pos = 0.0f;
  _limit_min_hit = true;

  // Move a bit away from the limit switch
  _hitLIMSW(70, 5.0f);
  // Configure normal motion profile again
  const float max_speed_mmps = 90.0f;
  const float max_accel_mmps2 = 100.0f;
  _stepper.setMaxSpeed(_mmToSteps(max_speed_mmps));
  _stepper.setAcceleration(_mmToSteps(max_accel_mmps2));
}

void PitchController::_alignToFullStep() {
  if (_config.microsteps <= 1) return;

  long currentPos = _stepper.currentPosition();
  int microstepsPerFull = _config.microsteps;
  int remainder = currentPos % microstepsPerFull;

  if (remainder != 0) {
    // Move to nearest full step position
    int stepsToMove = (remainder < microstepsPerFull / 2) ? -remainder : (microstepsPerFull - remainder);

    _stepper.move(stepsToMove);
    while (_stepper.distanceToGo() != 0) {
      _stepper.run();
    }
  }
}

void PitchController::setDivePhase(DivePhase phase) {
  _current_phase = phase;
}

void PitchController::setTargetPitch(float pitch_deg) {  // +ve for pitching up and -ve for pitching down
  _enableMotor();
  float pitch_rad = pitch_deg * M_PI / 180.0f;
  float target_mass_pos = _pitchToMassPosition(pitch_rad);

  // Apply mechanical constraints
  target_mass_pos = constrain(target_mass_pos, 0.0f, _config.max_travel);
  Serial.println(_mmToSteps(target_mass_pos));
  _stepper.moveTo(_mmToSteps(target_mass_pos));
}

void PitchController::update() {
  // Check limit switches
  _limit_min_hit = !digitalRead(_config.limit_switch_min_pin);
  _limit_max_hit = !digitalRead(_config.limit_switch_max_pin);

  if (_limit_min_hit) {
    _stepper.stop();
    _stepper.setCurrentPosition(0);
    _current_mass_pos = 0.0f;
    _hitLIMSW(70, 5.0f);
  } else if (_limit_max_hit) {
    _stepper.stop();
    _stepper.setCurrentPosition(_mmToSteps(_config.max_travel));
    _current_mass_pos = _config.max_travel;
    _hitLIMSW(-70, _current_mass_pos - 5.0f);
  }else if (_stepper.distanceToGo() != 0) {
    _stepper.run();
    if (digitalRead(_config.sleep_pin)) {  // Only update position if motor is awake
      _current_mass_pos = _stepper.currentPosition() / _mmToSteps(1.0f);
    }
  } else {
    _stepper.stop();
  }
}

void PitchController::emergencyStop() {
  _stepper.stop();
  _safeSleep();
}

float PitchController::getCurrentPitch() const {
  // Convert current mass position to CG position (mm from tail)
  float cg_pos = _current_mass_pos + _config.mechanism_start;

  if (_current_phase == DivePhase::ASCENDING) {
    // Convert CG position to pitch using ascending model
    float y_cm = (_config.cob_position - cg_pos) / 10.0f;  // mm to cm
    float pitch_rad = ASCEND_P1 * y_cm * y_cm + ASCEND_P2 * y_cm + ASCEND_P3;
    return pitch_rad * 180.0f / M_PI;
  } else {
    // Convert CG position to pitch using descending model
    float y_cm = (cg_pos - _config.cob_position) / 10.0f;  // mm to cm
    float pitch_rad = DESCEND_SLOPE * y_cm + DESCEND_INTERCEPT;
    return pitch_rad * 180.0f / M_PI;
  }
}

float PitchController::getCurrentMassPosition() const {
  return _current_mass_pos;
}

bool PitchController::getStepsToGo() {
  return _stepper.distanceToGo();
}

PitchController::DivePhase PitchController::getCurrentPhase() const {
  return _current_phase;
}

float PitchController::_pitchToMassPosition(float pitch_rad) const {
  float cg_pos_mm;

  if (_current_phase == DivePhase::ASCENDING) {
    cg_pos_mm = _ascendingModel(pitch_rad);
    // Constrain between COP and COB
    cg_pos_mm = constrain(cg_pos_mm, _config.cop_position, _config.cob_position);
  } else {
    cg_pos_mm = _descendingModel(pitch_rad);
    // Constrain 10-50mm ahead of COB
    cg_pos_mm = constrain(cg_pos_mm,
                          _config.cob_position + 10.0f,
                          _config.cob_position + 50.0f);
  }

  // Convert CG position to mass position // requires testing
  return cg_pos_mm - _config.mechanism_start;
}

float PitchController::_ascendingModel(float pitch_rad) const {
  // Solve quadratic equation for ascending phase
  float discriminant = ASCEND_P2 * ASCEND_P2 - 4 * ASCEND_P1 * (ASCEND_P3 - pitch_rad);
  float y_cm = (-ASCEND_P2 + sqrt(discriminant)) / (2 * ASCEND_P1);
  return _config.cob_position - (y_cm * 10.0f);  // cm to mm
}

float PitchController::_descendingModel(float pitch_rad) const {
  // Solve linear equation for descending phase
  float y_cm = (pitch_rad - DESCEND_INTERCEPT) / DESCEND_SLOPE;
  return _config.cob_position + (y_cm * 10.0f);  // cm to mm
}

long PitchController::_mmToSteps(float position_mm) const {
  float revolutions = position_mm / _config.lead_screw_pitch;
  return round(revolutions * _config.steps_per_rev * _config.microsteps);  // 1/32 microstepping
}

void PitchController::_enableMotor() {
  digitalWrite(_config.sleep_pin, HIGH);
  delay(1);  // Ensure driver is awake
}

void PitchController::_safeSleep() {
  _alignToFullStep();
  digitalWrite(_config.sleep_pin, LOW);
  delay(1);  // Ensure sleep takes effect
}