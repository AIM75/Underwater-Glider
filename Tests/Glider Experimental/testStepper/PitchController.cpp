#include "PitchController.h"

PitchController::PitchController(uint8_t stepPin, uint8_t dirPin, 
                               uint16_t stepsPerRev, float maxTravel, 
                               float leadScrewPitch) :
    _stepper(AccelStepper::DRIVER, stepPin, dirPin),
    _stepsPerRev(stepsPerRev),
    _maxTravel(maxTravel),
    _leadScrewPitch(leadScrewPitch),
    _currentPosition(0),
    _targetPosition(0)
{
}

void PitchController::begin() {
    // Configure stepper motor
    _stepper.setMaxSpeed(1000);       // steps/sec
    _stepper.setAcceleration(500);    // steps/sec²
    _stepper.setCurrentPosition(0);   // Reset position
}

void PitchController::setTargetPitch(float targetPitch) {
    // Constrain input pitch angle
    targetPitch = constrain(targetPitch, -30.0f, 30.0f);
    
    // Convert pitch angle to mass position
    _targetPosition = _pitchToPosition(targetPitch);
    
    // Constrain to physical limits
    _targetPosition = constrain(_targetPosition, -_maxTravel/2, _maxTravel/2);
    
    // Move stepper to new position
    _stepper.moveTo(_mmToSteps(_targetPosition));
}

void PitchController::update() {
    if (_stepper.distanceToGo() != 0) {
        _stepper.run();
        _currentPosition = _stepper.currentPosition() / _mmToSteps(1.0f);
    }
}

float PitchController::getCurrentPosition() const {
    return _currentPosition;
}

float PitchController::_pitchToPosition(float pitch) const {
    // This is where you implement your specific physics model
    // For a simple linear approximation:
    const float mmPerDegree = 0.5f; // Adjust based on your glider's CG characteristics
    return pitch * mmPerDegree;
}

long PitchController::_mmToSteps(float mm) const {
    float revolutions = mm / _leadScrewPitch;
    return revolutions * _stepsPerRev;
}