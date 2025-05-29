#include "MPU6050dmp.h"
#include <Wire.h>

// Constructor implementation
MPU6050DMP::MPU6050DMP(byte devAddr, int sdaPin, int sclPin) 
    : mpu(devAddr), devAddr(devAddr), sdaPin(sdaPin), sclPin(sclPin), 
      dmpReady(false), packetSize(0) {}

bool MPU6050DMP::begin() {
    // Initialize I2C communication
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(400000);  // Fast mode (400kHz)
    
    // Initialize MPU6050
    mpu.initialize();
    
    // Initialize DMP (Digital Motion Processor)
    byte devStatus = mpu.dmpInitialize();

    // Perform sensor calibration
    Calibration();

    // Check if DMP initialization was successful
    if (devStatus == 0) {
        mpu.setDMPEnabled(true);
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
        return true;
    }
    return false;
}

bool MPU6050DMP::update() {
    // Check if DMP is ready
    if (!dmpReady) return false;

    // Buffer for FIFO data
    byte fifoBuffer[64];
    
    // Check for new data and read if available
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
        // Extract quaternion from FIFO buffer
        mpu.dmpGetQuaternion(&Q, fifoBuffer);
        // Read raw acceleration data
        mpu.getAcceleration(&ax, &ay, &az);
        // Read raw gyro data
        mpu.getRotation(&p, &q, &r);
        return true;
    }
    return false;
}

void MPU6050DMP::getQuaternion(float &w, float &x, float &y, float &z) const {
    // Return quaternion components
    w = Q.w;
    x = Q.x;
    y = Q.y;
    z = Q.z;
}

void MPU6050DMP::getAcceleration(float &x, float &y, float &z) const {
    // Convert raw acceleration to g units (16384 LSB/g for ±2g range)
    x = ax / 16384.0f;
    y = ay / 16384.0f;
    z = az / 16384.0f;
}
void MPU6050DMP::getAngRates(float &x, float &y, float &z) const {
    // Convert raw acceleration to g units (16384 LSB/g for ±2g range)
    x =p;
    y =q;
    z =r;
}

bool MPU6050DMP::isReady() const {
    return dmpReady;
}

void MPU6050DMP::Calibration() { // yet to be finalized
    // Calibrate accelerometer (6 samples)
    mpu.CalibrateAccel(6);
    // Calibrate gyroscope (6 samples)
    mpu.CalibrateGyro(6);
}