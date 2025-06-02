#ifndef BALLASTSERVO_H
#define BALLASTSERVO_H

#include <Arduino.h>
#include <Servo.h>

class BallastServo {
public:
    /**
     * @brief Constructor
     * @param pin GPIO pin connected to servo
     * @param minPulse Minimum pulse width in microseconds (default 1000)
     * @param maxPulse Maximum pulse width in microseconds (default 2000)
     */
    BallastServo(uint8_t pin, uint16_t minPulse = 1000, uint16_t maxPulse = 2000);

    /**
     * @brief Initialize the servo
     */
    void begin();

    /**
     * @brief Set ballast position (-100 to 100)
     * @param position Negative values for descent, positive for ascent
     */
    void setPosition(int8_t position);

    /**
     * @brief Get current ballast position
     * @return Current position (-100 to 100)
     */
    int8_t getPosition() const;

private:
    Servo _servo;
    uint8_t _pin;
    uint16_t _minPulse;
    uint16_t _maxPulse;
    int8_t _currentPosition;
    const int8_t _minPosition = -100;
    const int8_t _maxPosition = 100;
};

#endif
