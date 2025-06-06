#include "Orientation.h"
#include <math.h>

// Initialize with zero angles
Orientation::Orientation()
  : roll(0), pitch(0), yaw(0) {}

bool Orientation::begin() {
  // Initialize the IMU
  return imu.begin();
}

bool Orientation::update() {
  // Check for new IMU data
  if (!imu.update()) return false;

  // Get current quaternion from IMU
  float qw, qx, qy, qz;
  imu.getQuaternion(qw, qx, qy, qz);

  // Calculate Euler angles from quaternion
  Qt2Ea(qw, qx, qy, qz);

  return true;
}

float Orientation::getRoll() const {
  return roll;
}

float Orientation::getPitch() const {
  return pitch;
}

float Orientation::getYaw() const {
  return yaw;
}

bool Orientation::isReady() const {
  // Check if IMU is ready
  return imu.isReady();
}

void Orientation::Qt2Ea(float qw, float qx, float qy, float qz) {
  double sqw = qw * qw;
  double sqx = qx * qx;
  double sqy = qy * qy;
  double sqz = qz * qz;

  yaw = atan2(2.0 * (qx * qy + qz * qw), (sqx - sqy - sqz + sqw));
  pitch = asin(2.0 * (qx * qz - qy * qw) / (sqx + sqy + sqz + sqw));
  roll = atan2(2.0 * (qy * qz + qx * qw), (-sqx - sqy + sqz + sqw));

  const float rad2deg = 180.0f / PI;
  roll *= rad2deg;
  pitch *= rad2deg;
  yaw *= rad2deg;
}