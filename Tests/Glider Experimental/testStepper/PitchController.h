#ifndef PITCHCONTROLLER_H
#define PITCHCONTROLLER_H

#include <AccelStepper.h>

class PitchController {
public:
    /**
     * @brief Constructor for pitch controller
     * @param stepPin Stepper motor step pin
     * @param dirPin Stepper motor direction pin
     * @param stepsPerRev Steps per revolution of the stepper
     * @param maxTravel Maximum linear travel distance of mass (mm)
     * @param leadScrewPitch Lead screw pitch (mm/revolution)
     */
    PitchController(uint8_t stepPin, uint8_t dirPin, uint16_t stepsPerRev, 
                  float maxTravel, float leadScrewPitch);

    /**
     * @brief Initialize the pitch controller
     */
    void begin();

    /**
     * @brief Set target pitch angle
     * @param targetPitch Desired pitch angle in degrees (-30 to 30)
     */
    void setTargetPitch(float targetPitch);

    /**
     * @brief Run the controller (call in loop)
     */
    void update();

    /**
     * @brief Get current mass position
     * @return Position in mm from center
     */
    float getCurrentPosition() const;

private:
    AccelStepper _stepper;
    const uint16_t _stepsPerRev;
    const float _maxTravel;
    const float _leadScrewPitch;
    float _currentPosition; // mm from center
    float _targetPosition;  // mm from center
    
    /**
     * @brief Convert pitch angle to mass position
     * @param pitch Angle in degrees
     * @return Required mass position in mm
     */
    float _pitchToPosition(float pitch) const;
    
    /**
     * @brief Convert mm position to steps
     * @param mm Position in millimeters
     * @return Number of steps
     */
    long _mmToSteps(float mm) const;
};

#endif