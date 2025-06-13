#ifndef APPLICATION_H
#define APPLICATION_H
// Include section
#include "Orientation.h"
#include "MPU6050dmp.h"
#include "DepthModule.h"
#include "BallastServo.h"
#include "sdCardModule.h"
#include "CommunicationModule.h"
#include "PitchController.h"
#include <ESP32Servo.h>
#include <ESP32PWM.h>


//-------------------------------------------------------Configurations------------------------------------

//--------------------------------------------------------Definitions------------------------------------

#define _mpxPIN 35     // MPX5010 pressure sensor pin
#define _servoPIN 32   // servo motor signal pin
#define _stepPIN 26    // stepper driver STEP pin
#define _dirPIN 33     // stepper driver DIR pin
#define _slpPIN 27     // stepper driver DIR pin
#define _limSW1PIN 14  // Limit switch 1 pin
#define _limSW2PIN 12  // Limit switch 2 pin

#define _surfaceDepth 2.0f   // in cm
#define _maxDepth 10.0f      // in cm
#define _defaultTHEAT 15.0f  // in deg
#define _ballastOPEN 150     // in deg
#define _ballastCLOSED 60    // in deg


PitchConfig config = {
  // Pin configuration
  .step_pin = _stepPIN,
  .dir_pin = _dirPIN,
  .sleep_pin = _slpPIN,
  .limit_switch_min_pin = _limSW2PIN,
  .limit_switch_max_pin = _limSW1PIN,

  // Mechanical properties
  .steps_per_rev = 200,
  .lead_screw_pitch = 8.0f,  // 8mm lead screw
  .max_travel = 65.0f,      // 100mm max travel

  // Physics parameters (mm from tail)
  .cob_position = 205.55f,   // Center of buoyancy
  .cop_position = 199.5f,    // Center of pressure
  .mechanism_start = 27.0f,  // Mass mechanism start position

  // Driver configuration
  .microsteps = 32  // 1/32 microstepping
};

//-----------------------------------------------------------WIFI----------------------------------------------

const char* ssid = "AIM-MIA";
const char* password = "123456789";
IPAddress local_ip(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
const uint16_t wifi_port = 42050;

// -----------------------------------------------------Module instances--------------------------------------------

MPX5010 depthSensor(_mpxPIN);
Orientation orientation;
SDLogger sdCard;
BallastServo ballast(_servoPIN);
WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);
PitchController pitchController(config);

//------------------------------------------------------------Variables-------------------------

enum GliderState { SURFACE_COMNS,
                   DESCENDING,
                   ASCENDING,
                   EMERGENCY };
GliderState currentState = SURFACE_COMNS;

float targetPitchU = _defaultTHEAT;   // Default pitch angle
float targetPitchD = -_defaultTHEAT;  // Default pitch angle
float maxDepth = _maxDepth;           // Default max depth (cm)

float depth;
float pitch;
float roll;
int8_t ballastPos;

// ------------------------------------------------------Functions Declarations-----------------------------------------
bool initializeModules();
void updateSensorData();
void checkSurface();
void handleSurfaceOperations();
void parseDiveParameters(String cmd);
void processCommand(String cmd);
void runStateMachine();
void emergencyProcedure();

//----------------------------------------------Functions Definitions----------------------------------------


bool initializeModules() {
  depthSensor.begin();
  wifiComms.begin();
  orientation.begin();
  sdCard.begin();
  ballast.begin();
  pitchController.begin();
  return true;
}


void updateSensorData() {

  depth = depthSensor.readDepthCm();
  pitch = -101;
  roll = -101;
  if (orientation.update()) {
    pitch = orientation.getPitch();
    roll = orientation.getRoll();
  }
  ballastPos = ballast.getPosition();

  String data = String(millis()) + "," + String(depth, 2) + "," + String(pitch, 1) + "," + String(roll, 1) + "," + String(ballastPos) + ";";
  Serial.println(data);
  sdCard.logData(data);
}

void runStateMachine() {
  depth = depthSensor.readDepthCm();

  switch (currentState) {
    Serial.println("Surface");
    case SURFACE_COMNS:
      while (currentState == SURFACE_COMNS) {
        checkSurface();
      }
      break;

    case DESCENDING:
      Serial.println("Descending");
      pitchController.setDivePhase(PitchController::DivePhase::DESCENDING);
      pitchController.setTargetPitch(targetPitchD);
      while (pitchController.getStepsToGo() != 0) {
        pitchController.update();
        // Serial.println("Running Descending");
      }
      ballast.setPosition(_ballastOPEN);
      if (depth > maxDepth) {
        currentState = ASCENDING;
      }
      break;

    case ASCENDING:
      Serial.println("Ascending");
      ballast.setPosition(_ballastCLOSED);
      pitchController.setDivePhase(PitchController::DivePhase::ASCENDING);
      pitchController.setTargetPitch(targetPitchU);
      while (pitchController.getStepsToGo() != 0) {
        pitchController.update();
        Serial.println("Running Ascending");
      }
      if (depth < _surfaceDepth) {
        currentState = SURFACE_COMNS;
      }
      break;

    case EMERGENCY:
      emergencyProcedure();
      break;
  }
}

void checkSurface() {
  depth = depthSensor.readDepthCm();

  if (depth < _surfaceDepth) {
    if (!(wifiComms.isConnected())) {
      wifiComms.begin();
    } else {
      handleSurfaceOperations();
    }
  }
}


void handleSurfaceOperations() {
  // Ensure SD card is available
  // if (!sdCard.isAvailable()) {
  //   Serial.println("SD card not available for data transmission");
  //   return;
  // }

  // // Send all stored data line by line
  // String dataLine;
  // while (((dataLine = sdCard.readNextLine()).length() > 0) && wifiComms.isConnected()) {
  //   wifiComms.sendData(dataLine);
  //   Serial.println("sent");
  //   delay(10);  // Small delay between transmissions
  // }

  // Clear data file after successful transmission
  // if (SD.exists(sdCard.getFileName())) {
  //   SD.remove(sdCard.getFileName());
  //   Serial.println("Cleared data file after transmission");
  // }

  // Process incoming commands
  String cmd = wifiComms.receiveCommand();
  if (cmd.length() > 0) {
    processCommand(cmd);
  }
}


void processCommand(String cmd) {
  // Command format: "BEGIN_DIVE:PITCHD:-15:PITCHU:15:DEPTH:5;"
  if (cmd.startsWith("BEGIN_DIVE")) {
    parseDiveParameters(cmd);
    currentState = DESCENDING;
    wifiComms.end();  // Power down WiFi
  } else if (cmd.startsWith("CHGPIT")) {
    parseDiveParameters(cmd);
    currentState = DESCENDING;
  }
}

void parseDiveParameters(String cmd) {
  int pitchDIndex = cmd.indexOf("PITCHD:");
  int pitchUIndex = cmd.indexOf("PITCHU:");
  int depthIndex = cmd.indexOf("DEPTH:");

  if (pitchDIndex != -1) {
    targetPitchU = cmd.substring(pitchDIndex + 7, cmd.indexOf(':', pitchDIndex + 7)).toFloat();
  }
  if (pitchUIndex != -1) {
    targetPitchD = cmd.substring(pitchUIndex + 7, cmd.indexOf(':', pitchUIndex + 7)).toFloat();
  }
  if (depthIndex != -1) {
    maxDepth = cmd.substring(depthIndex + 6).toFloat();
  }
}


void emergencyProcedure() {
}

#endif
