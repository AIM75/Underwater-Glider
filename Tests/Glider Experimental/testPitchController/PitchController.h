#ifndef PITCH_CONTROLLER_H
#define     PITCH_CONTROLLER_H

#include <AccelStepper.h>

struct PitchConfig {
    // Pin configuration
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t sleep_pin;
    
    // Mechanical properties (all in mm)
    uint16_t steps_per_rev = 200;     // NEMA 17 steps/revolution
    float lead_screw_pitch = 8.0f;    // Lead screw pitch (mm/rev)
    float max_travel = 20.0f;        // Maximum linear travel (mm)
    
    // Glider physics (positions relative to tail in mm)
    float cob_position = 205.55f;      // Center of buoyancy
    float cop_position = 199.5f;      // Center of pressure
    float mechanism_start = 50.0f;    // Start position of movable mass

    // Microstepping configuration
    uint8_t microsteps = 32;
};

class PitchController {
public:
    enum class DivePhase { ASCENDING, DESCENDING };
    
    PitchController(const PitchConfig& config);
    
    void begin();
    void setDivePhase(DivePhase phase);
    void setTargetPitch(float pitch_deg);
    void update();
    void emergencyStop();
    
    float getCurrentPitch() const;
    float getCurrentMassPosition() const;  // Returns position in mm
    bool isMoving();
    DivePhase getCurrentPhase() const;

private:
    const PitchConfig _config;
    AccelStepper _stepper;
    DivePhase _current_phase;
    float _current_mass_pos;  // Current mass position in mm
    
    // Physics model coefficients
    static constexpr float ASCEND_P1 = -0.0025f;  // θ = p1*y² + p2*y + p3
    static constexpr float ASCEND_P2 = 0.157f;
    static constexpr float ASCEND_P3 = 0.0785f;
    static constexpr float DESCEND_SLOPE = -1.697f;  // θ = 1.697*y + 0.1217
    static constexpr float DESCEND_INTERCEPT = -0.1217f;

    // Core calculations
    float _pitchToMassPosition(float pitch_rad) const;
    float _ascendingModel(float pitch_rad) const;
    float _descendingModel(float pitch_rad) const;
    long _mmToSteps(float position_mm) const;
    void _enableMotor();
    void _safeSleep();
    void _alignToFullStep();
};

#endif