// we have problems when the pitch angle reaches 90 deg the roll and yaw go mad

#ifndef ORIENTATION_H
#define ORIENTATION_H

#include "MPU6050dmp.h"

/**
 * @class OrientationCalculator
 * @brief Provides high-level orientation data from IMU
 * 
 * Wraps the MPU6050_DMP class to provide easy-to-use orientation angles
 * in degrees (roll, pitch, yaw) and handles all quaternion conversions
 */
class Orientation{
public:
    /**
     * @brief Constructor - initializes with default settings
     */
    Orientation();
    
    /**
     * @brief Initialize the sensor and DMP
     * @return true if initialization succeeded, false otherwise
     */
    bool begin();
    
    /**
     * @brief Update orientation calculations with new sensor data
     * @return true if new data was processed, false otherwise
     */
    bool update();
    
    /**
     * @brief Get current roll angle (X-axis rotation)
     * @return Roll angle in degrees (-180 to 180)
     */
    float getRoll() const;
    
    /**
     * @brief Get current pitch angle (Y-axis rotation)
     * @return Pitch angle in degrees (-90 to 90)
     */
    float getPitch() const;
    
    /**
     * @brief Get current yaw angle (Z-axis rotation)
     * @return Yaw angle in degrees (-180 to 180)
     */
    float getYaw() const;

    
    /**
     * @brief Check if sensor is ready
     * @return true if ready, false otherwise
     */
    bool isReady() const;


    void getAcceleration(float& ax, float& ay, float& az) const;

private:
    MPU6050DMP imu;  // MPU6050 interface instance
    
    // Current orientation angles (degrees)
    float roll;   // X-axis rotation (phi)
    float pitch;  // Y-axis rotation (theta)
    float yaw;    // Z-axis rotation (psi)
    
    /**
     * @brief Convert quaternion to Euler angles
     * @param qw Quaternion w component
     * @param qx Quaternion x component
     * @param qy Quaternion y component
     * @param qz Quaternion z component
     */
    void Qt2Ea(float qw, float qx, float qy, float qz);
};

#endif