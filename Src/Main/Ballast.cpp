#include "Ballast.h"

Ballast::Ballast(uint8_t pin) :
    _pin(pin),
    _currentPosition(0)
{
    pinMode(_pin,OUTPUT);
}

void Ballast::begin(int pos) {
    ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	_servo.setPeriodHertz(50); 
    
    _servo.attach(_pin);
    setPosition(pos); // Start at Closed position
}

void Ballast::setPosition(int8_t position) {    
    _servo.write(position);
    _currentPosition = position;
}

int8_t Ballast::getPosition() const {
    return _currentPosition;
}
