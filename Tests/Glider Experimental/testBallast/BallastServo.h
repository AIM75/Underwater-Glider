#ifndef BALLASTSERVO_H
#define BALLASTSERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

class BallastServo {
public:
    /**
     * @brief Constructor
     * @param pin GPIO pin connected to servo
     * @param minPulse Minimum pulse width in microseconds (default 1000)
     * @param maxPulse Maximum pulse width in microseconds (default 2000)
     */
    BallastServo(uint8_t pin);

    /**
     * @brief Initialize the servo
     */
    void begin(int pos);

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
    int8_t _currentPosition;
};

#endif
