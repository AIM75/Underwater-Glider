#include "BallastServo.h"

BallastServo::BallastServo(uint8_t pin, uint16_t minPulse, uint16_t maxPulse) :
    _pin(pin),
    _minPulse(minPulse),
    _maxPulse(maxPulse),
    _currentPosition(0)
{
}

void BallastServo::begin() {
    _servo.attach(_pin, _minPulse, _maxPulse);
    setPosition(0); // Start at neutral position
}

void BallastServo::setPosition(int8_t position) {
    // Constrain input to valid range
    position = constrain(position, _minPosition, _maxPosition);
    
    // Map position to servo angle (0-180 degrees)
    uint8_t angle = map(position, _minPosition, _maxPosition, 0, 180);
    
    _servo.write(angle);
    _currentPosition = position;
}

int8_t BallastServo::getPosition() const {
    return _currentPosition;
}
