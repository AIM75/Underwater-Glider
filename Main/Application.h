#ifndef APPLICATION_H
#define APPLICATION_H
// Include section
#include "Orientation.h"
#include "MPU6050dmp.h"
#include "DepthModule.h"
#include "BallastServo.h"
#include "sdCardModule.h"
#include "CommunicationModule.h"
#include <ESP32Servo.h>
#include <ESP32PWM.h>


//-------------------------------------------------------Configurations------------------------------------

//--------------------------------------------------------Definitions------------------------------------

#define _depthPIN 13
#define _servoPIN 25
#define _surfaceDepth 0.1f   // in meter
#define _maxDepth 1.0f       // in meter
#define _defaultTHEAT 15.0f  // in deg
#define _ballastOPEN 150     // in deg
#define _ballastCLOSED 60    // in deg

//-----------------------------------------------------------WIFI----------------------------------------------

const char* ssid = "AIM-MIA";
const char* password = "123456789";
IPAddress local_ip(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 5);
IPAddress subnet(255, 255, 255, 0);
const uint16_t wifi_port = 42050;

// -----------------------------------------------------Module instances--------------------------------------------

MPX5010 depthSensor(_depthPIN);
Orientation orientation;
SDLogger sdCard;
BallastServo ballast(_servoPIN);
WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);

//------------------------------------------------------------Variables-------------------------

enum GliderState { SURFACE_COMNS,
                   DESCENDING,
                   ASCENDING,
                   EMERGENCY };
GliderState currentState = SURFACE_COMNS;

float targetPitch = _defaultTHEAT;  // Default pitch angle
float maxDepth = _maxDepth;         // Default max depth (meters)
bool isAtSurface = false;

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
  orientation.begin();
  sdCard.begin();
  ballast.begin();
  wifiComms.begin();
  return true;
}


void updateSensorData() {

  float depth = depthSensor.readDepthCm() / 100.0f;
if (orientation.update()){
  float pitch = orientation.getPitch();
  float roll = orientation.getRoll();
}
  int8_t ballastPos = ballast.getPosition();

  String data = String(millis()) + "," + String(depth, 2) + "," + String(pitch, 1) + "," + String(roll, 1) + "," + String(ballastPos);

  if (!isAtSurface) {
    sdCard.logData(data);
  }
}

void runStateMachine() {
  float currentDepth = depthSensor.readDepthCm() / 100.0f;

  switch (currentState) {
    case SURFACE_COMNS:
      ballast.setPosition(_ballastCLOSED);
      checkSurface();
      break;

    case DESCENDING:
      ballast.setPosition(_ballastOPEN);
      if (currentDepth > maxDepth) {
        currentState = ASCENDING;
      }
      break;

    case ASCENDING:
      ballast.setPosition(_ballastCLOSED);
      if (currentDepth < _surfaceDepth) {
        currentState = SURFACE_COMNS;
      }
      break;

    case EMERGENCY:
      emergencyProcedure();
      break;
  }
}

void checkSurface() {
  float depth = depthSensor.readDepthCm() / 100.0f;

  if (depth < _surfaceDepth) {
    if (!isAtSurface) {
      wifiComms.begin();
      handleSurfaceOperations();
    }
    isAtSurface = true;
  } else {
    isAtSurface = false;
  }
}


void handleSurfaceOperations() {
  // Send all SD card data
  /*
  File dataFile = sdCard.openDataFile();
  while (dataFile.available()) {
    wifiComms.sendData(dataFile.readStringUntil('\n'));
  }
  dataFile.close();
  sdCard.clearDataFile();
  */

  // Process commands
  String cmd = wifiComms.receiveCommand();
  if (cmd.length() > 0) {
    processCommand(cmd);
  }
}


void parseDiveParameters(String cmd) {
  int pitchIndex = cmd.indexOf("PITCH:");
  int depthIndex = cmd.indexOf("DEPTH:");

  if (pitchIndex != -1) {
    targetPitch = cmd.substring(pitchIndex + 6, cmd.indexOf(':', pitchIndex + 6)).toFloat();
  }
  if (depthIndex != -1) {
    maxDepth = cmd.substring(depthIndex + 6).toFloat();
  }

  Serial.print("Dive params - Pitch: ");
  Serial.print(targetPitch);
  Serial.print(", Max Depth: ");
  Serial.println(maxDepth);
}


void processCommand(String cmd) {
  // Command format: "BEGIN_DIVE:PITCH:15:DEPTH:5;"
  if (cmd.startsWith("BEGIN_DIVE")) {
    parseDiveParameters(cmd);
    currentState = DESCENDING;
    wifiComms.end();  // Power down WiFi
  }
}

void emergencyProcedure() {
}

#endif
