#ifndef BALLAST_H
#define BALLAST_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <ESP32PWM.h>

class Ballast {
public:
    /**
     * @brief Constructor
     * @param pin GPIO pin connected to servo
     */
    Ballast(uint8_t pin);

    /**
     * @brief Initialize the servo
     */
    void begin(int pos);

    /**
     * @brief Set ballast position 
     * @param position for servo is 0 - 180
     */
    void setPosition(int8_t position);

    /**
     * @brief Get current ballast position
     * @return Current position (0 to 180)
     */
    int8_t getPosition() const;

private:
    Servo _servo;
    uint8_t _pin;
    int8_t _currentPosition;
};

#endif
