#ifndef MPX5010_H
#define MPX5010_H

#include <Arduino.h>

class MPX5010 {
public:
    // Constructor
    MPX5010(uint8_t pin);
    
    // Initialize the sensor
    void begin();
    
    // Read pressure in kPa
    float readPressurekPa();
    
    // Read pressure in cmH2O
    float readPressureCmH2O();
    
    // Calculate water depth in cm
    float readDepthCm();
    
    // Calibration function (call when sensor is at known pressure)
    void calibrate(float knownPressurekPa);
    
    // Set custom calibration values
    void setCalibration(float offset, float sens);
    
private:
    uint8_t _pin;
    float _sensorOffset;  // in mV
    float _sensitivity;   // in mV/mmH2O
    const float _ADC_mV = 3.3/4095.0;       // conversion multiplier from ADC value to voltage in mV which is the max voltage by the max steps
    const float _mmh2O_cmH2O = 10;         // divide by this figure to convert mmH2O to cmH2O
    const float _mmh2O_kpa = 0.00981;      // conversion multiplier from mmH2O to kPa
    const float _gravity = 9.80665;         // standard gravity m/s²
};

#endif