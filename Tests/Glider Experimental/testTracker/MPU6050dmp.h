#ifndef MPU6050DMP_h
#define MPU6050DMP_h

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

class MPU6050DMP {
public:
    /**
     * @brief Constructor
     * @param devAddr I2C address of the MPU6050 (default 0x68)
     * @param sdaPin GPIO pin for I2C SDA (default 21)
     * @param sclPin GPIO pin for I2C SCL (default 22)
     */
    MPU6050DMP(byte devAddr = 0x68, int sdaPin = 21, int sclPin = 22);
    
    /**
     * @brief Initialize the MPU6050 and DMP
     * @return true if initialization succeeded, false otherwise
     */
    bool begin();
    
    /**
     * @brief Read new data from the sensor
     * @return true if new data was available, false otherwise
     */
    bool update();
    
    /**
     * @brief Get the current quaternion values
     * @param w Reference to store the w component
     * @param x Reference to store the x component
     * @param y Reference to store the y component
     * @param z Reference to store the z component
     */
    void getQuaternion(float &w, float &x, float &y, float &z) const;
    
    /**
     * @brief Get the current acceleration values in g units
     * @param x Reference to store x-axis acceleration
     * @param y Reference to store y-axis acceleration
     * @param z Reference to store z-axis acceleration
     */
    void getAcceleration(float &x, float &y, float &z) const;

    void getAngRates(float &x, float &y, float &z) const;
    
    /**
     * @brief Check if DMP is ready
     * @return true if DMP is ready, false otherwise
     */
    bool isReady() const;

private:
    /**
     * @brief Perform calibration of accelerometer and gyroscope
     */
    void Calibration();

    // MPU6050 sensor instance
    MPU6050 mpu;
    
    // Configuration parameters
    byte devAddr;  // I2C device address
    int sdaPin;    // I2C SDA pin
    int sclPin;    // I2C SCL pin
    
    // DMP status flags
    bool dmpReady;     // True if DMP is initialized
    int packetSize;    // Size of DMP data packet
    
    // Sensor data
    Quaternion Q;      // Quaternion data from DMP
    int16_t ax, ay, az, p , q , r; // Raw acceleration data
};

#endif