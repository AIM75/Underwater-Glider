#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include "Orientation.h"
#include "DepthModule.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

class PositionTracker {
public:
  PositionTracker(Orientation& orientation, MPX5010& depthSensor);
  ~PositionTracker();

  void begin(uint8_t taskPriority = 5, BaseType_t coreID = PRO_CPU_NUM);
  void end();

  void calibrateAccelerometer(uint32_t calibrationTime = 1000);
  void setAccelerometerOffsets(float ax, float ay, float az);
  // Thread-safe state access
  float getX() const;

  // Configuration (thread-safe)
  void setHighPassCutoff(float cutoff_freq);
  void setZUPTThreshold(float threshold);

private:
  // Sensor references
  Orientation& _orient;
  MPX5010& _depthSensor;

  // RTOS components
  TaskHandle_t _taskHandle = nullptr;
  mutable SemaphoreHandle_t _mutex;

  // Filter and state
  struct {
    float x = 0;
    float vx = 0;
    float z = 0;
    float vz = 0;
    float prev_z = 0;
    float ax_filtered = 0;
    float az_filtered = 0;
    float alpha = 0.1f;
    float zupt_threshold = 0.01f;
  } _state;

  // Calibration offsets
  float _ax_offset;
  float _ay_offset;
  float _az_offset;

  // Constants
  const float _g = 9.80665f;
  uint32_t _last_update = 0;

  // Internal methods
  static void _updateTask(void* params);
  void _updateImpl();
  void _updateHighPassFilter(float ax, float az, float dt);
  void _transformAcceleration(float& ax, float& az, float pitch);
};
#endif