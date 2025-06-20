#include "DepthModule.h"

// Constructor with default calibration values
MPX5010::MPX5010(uint8_t pin) : 
    _pin(pin),
    _sensorOffset((0.04+_PerorrKpa * _tempMultiplier * _KpaSensFactor)*_Vin),  // default offset from datasheet (mV)
    _sensitivity(_KpaSensFactor * _Vin)    // default sensitivity from datasheet (mV/Kpa)
{
}

void MPX5010::begin() {
    pinMode(_pin, INPUT);
}

float MPX5010::readPressurekPa() {
    float voltage_mV = (analogRead(_pin)) * _ADC_mV;
    float pressure_Kpa = (voltage_mV - _sensorOffset) / _sensitivity;
    return pressure_Kpa;
}

float MPX5010::readPressureCmH2O() {
    float voltage_mV = analogRead(_pin) * _ADC_mV;
    float pressure_Kpa = (voltage_mV - _sensorOffset) / _sensitivity;
    return (pressure_Kpa / _mmh2O_kpa)/_mmh2O_cmH2O ;
}

float MPX5010::readDepthCm() {
    float pressure_kPa = readPressurekPa();
    // depth = pressure / (density * gravity)
    // For water: density ~1000 kg/m³, gravity 9.80665 m/s²
    // kPa to Pa: *1000
    // Result in meters: *100 to cm
    return (pressure_kPa * 1000) / (1000 * _gravity) * 100;
}