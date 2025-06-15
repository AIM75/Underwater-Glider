#include "BallastServo.h"

BallastServo::BallastServo(uint8_t pin) :
    _pin(pin),
    _currentPosition(0)
{
}

void BallastServo::begin(int pos) {
    ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	_servo.setPeriodHertz(50); 
    _servo.attach(_pin, 1000, 2000);
    setPosition(pos); // Start at neutral position
}

void BallastServo::setPosition(int8_t position) {    
    _servo.write(position);
    _currentPosition = position;
}

int8_t BallastServo::getPosition() const {
    return _currentPosition;
}
