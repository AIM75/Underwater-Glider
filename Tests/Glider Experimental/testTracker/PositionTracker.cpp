#include "PositionTracker.h"
#include <cmath>

PositionTracker::PositionTracker(Orientation& orientation, MPX5010& depthSensor)
  : _orient(orientation), _depthSensor(depthSensor), _ax_offset(0), _ay_offset(0), _az_offset(0) {
  _mutex = xSemaphoreCreateMutex();
}

PositionTracker::~PositionTracker() {
  end();
  vSemaphoreDelete(_mutex);
}

void PositionTracker::begin(uint8_t taskPriority, BaseType_t coreID) {
  if (_taskHandle == nullptr) {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _state.z = _depthSensor.readDepthCm() / 100.0f;
    _state.prev_z = _state.z;
    _last_update = xTaskGetTickCount();
    xSemaphoreGive(_mutex);

    xTaskCreatePinnedToCore(
      _updateTask,   // Task function
      "PosTracker",  // Task name
      4096,          // Stack size
      this,          // Parameters
      taskPriority,  // Priority
      &_taskHandle,  // Task handle
      coreID         // Core ID
    );
  }
}

void PositionTracker::end() {
  if (_taskHandle) {
    vTaskDelete(_taskHandle);
    _taskHandle = nullptr;
  }
}

void PositionTracker::_updateTask(void* params) {
  PositionTracker* tracker = static_cast<PositionTracker*>(params);
  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    tracker->_updateImpl();
    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(20));  // 50Hz
  }
}

void PositionTracker::_updateImpl() {
  TickType_t now = xTaskGetTickCount();
  float dt = (now - _last_update) * portTICK_PERIOD_MS / 1000.0f;
  _last_update = now;

  if (dt <= 0 || dt > 0.1f) return;

  // Local copies for thread safety
  float ax, ay, az, pitch, z;

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    // Get sensor data
    if (!_orient.update()) {
      xSemaphoreGive(_mutex);
      return;
    }

    _orient.getAcceleration(ax, ay, az);
    ax -= _ax_offset;
    ay -= _ay_offset;
    az -= _az_offset;
    pitch = _orient.getPitch() * M_PI / 180.0f;
    z = _depthSensor.readDepthCm() / 100.0f;

    // Update filters
    _updateHighPassFilter(ax, az, dt);

    // Transform accelerations
    float ax_world = _state.ax_filtered;
    float az_world = _state.az_filtered;
    _transformAcceleration(ax_world, az_world, pitch);

    // Update state ----- Double integration
    _state.vx += ax_world * dt;
    _state.x += _state.vx * dt;
    _state.vz = (z - _state.prev_z) / dt;
    _state.z = z;
    _state.prev_z = z;

    // Apply ZUPT
    if (fabs(_state.vz) < _state.zupt_threshold) {
      _state.vx *= 0;  // Dampen velocity
    }

    xSemaphoreGive(_mutex);
  }
}

void PositionTracker::_updateHighPassFilter(float ax, float az, float dt) {
  static float ax_prev = 0, az_prev = 0;

  _state.ax_filtered = _state.alpha * (_state.ax_filtered + ax - ax_prev);
  _state.az_filtered = _state.alpha * (_state.az_filtered + az - az_prev);

  ax_prev = ax;
  az_prev = az;
}

void PositionTracker::_transformAcceleration(float& ax, float& az, float pitch) {
  float cosTheta = cos(pitch);
  float sinTheta = sin(pitch);
  ax = ax * cosTheta + az * sinTheta;
  az = -ax * sinTheta + az * cosTheta - _g;
}

// Thread-safe getters
float PositionTracker::getX() const {
  float val = 0;
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    val = _state.x;
    xSemaphoreGive(_mutex);
  }
  return val;
}
void PositionTracker::calibrateAccelerometer(uint32_t calibrationTime) {
  float ax_sum = 0, ay_sum = 0, az_sum = 0;
  uint32_t startTime = millis();
  uint32_t samples = 0;

  while (millis() - startTime < calibrationTime) {
    if (_orient.update()) {
      float ax, ay, az;
      _orient.getAcceleration(ax, ay, az);

      ax_sum += ax;
      ay_sum += ay;
      az_sum += az;
      samples++;

      delay(10);
    }
  }

  if (samples > 0) {
    _ax_offset = ax_sum / samples;
    _ay_offset = ay_sum / samples;
    _az_offset = (az_sum / samples) - 1.0f;
  }
}

void PositionTracker::setAccelerometerOffsets(float ax, float ay, float az) {
  _ax_offset = ax;
  _ay_offset = ay;
  _az_offset = az;
}

// Thread-safe configuration
void PositionTracker::setHighPassCutoff(float cutoff_freq) {
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    _state.alpha = 2 * M_PI * cutoff_freq * 0.02f;  // For 50Hz
    _state.alpha = fmax(0.01f, fmin(_state.alpha, 0.5f));
    xSemaphoreGive(_mutex);
  }
}

void PositionTracker::setZUPTThreshold(float threshold) {
  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    _state.zupt_threshold = threshold;
    xSemaphoreGive(_mutex);
  }
}