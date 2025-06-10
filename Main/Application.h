#ifndef APPLICATION_H
#define APPLICATION_H
// Include section
#include "Orientation.h"
#include "MPU6050dmp.h"
#include "DepthModule.h"
#include "BallastServo.h"
#include "PitchController.h"
#include "sdCardModule.h"
#include "CommunicationModule.h"
#include <ESP32Servo.h>
#include <ESP32PWM.h>


//-------------------------------------------------------Configurations------------------------------------

//--------------------------------------------------------Definitions------------------------------------

#define _mpxPIN 36     // MPX5010 pressure sensor pin
#define _servoPIN 32   // servo motor signal pin
#define _stepPIN 33    // stepper driver STEP pin
#define _dirPIN 25     // stepper driver DIR pin
#define _limSW1PIN 26  // Limit switch 1 pin
#define _limSW2PIN 27  // Limit switch 2 pin

#define _surfaceDepth 0.03f   // in meter
#define _minDepth 0.01f      // in meter
#define _maxDepth 1.0f       // in meter
#define _defaultTHEAT 15.0f  // in deg
#define _ballastOPEN 150     // in deg
#define _ballastCLOSED 60    // in deg

#define _defaultPitch 0.0f       // Default pitch angle (degrees)
#define _minPitch -30.0f         // Minimum pitch angle
#define _maxPitch 30.0f          // Maximum pitch angle
#define _surfaceThreshold 0.03f  // Surface detection threshold (3cm)

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
PitchController pitchController(_stepPIN, _dirPIN, 200, 100.0f, 2.0f);  // Steps/rev, max travel, lead screw pitch
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

float pitch = -101;
float roll = -101;
float depth = 0;
float diveDepth;
float currentDepth = 0;
int8_t ballastPos;
float massPosition;
float currentPitch = _defaultPitch;
bool dataSent = false;

// ------------------------------------------------------Functions Declarations-----------------------------------------
bool initializeModules();
void updateSensorData();
void checkSurface();
//void onSurface();
void handleSurfaceOperations();
void sendStoredData();
void parseDiveParameters(String cmd);
void processCommand(String cmd);
void runStateMachine();
void emergencyProcedure();


//----------------------------------------------Functions Definitions----------------------------------------

//-------------------------------------------Initialization and Updating-------------------------------------

bool initializeModules() {
  depthSensor.begin();
  orientation.begin();
  //sdCard.begin();
  ballast.begin();
  wifiComms.begin();
  return true;
}

void updateSensorData() {

  depth = depthSensor.readDepthCm() / 100.0f;
  pitch = -101;
  roll = -101;
  if (orientation.update()) {
    pitch = orientation.getPitch();
    roll = orientation.getRoll();
  }
  ballastPos = ballast.getPosition();
  massPosition = pitchController.getCurrentPosition();

  String data = String(millis()) + "," + String(depth, 3) + "," + String(pitch, 1) + "," + String(roll, 1) + "," + String(ballastPos) + "," + String(massPosition, 1) + "," + String(targetPitch, 1);

  Serial.println(data);
  Serial.println(analogRead(_mpxPIN));
  Serial.println("--------------------------------------------------------------------------------------");

  if (!isAtSurface) {
    sdCard.logData(data);
  }
}

//---------------------------------------------------State Machine-------------------------------------------

void runStateMachine() {
  currentDepth = depthSensor.readDepthCm() / 100.0f;

  switch (currentState) {
    case SURFACE_COMNS:
      ballast.setPosition(_ballastCLOSED);
      pitchController.setTargetPitch(0); // Level at surface
      pitchController.update();
      
      if (!dataSent && wifiComms.isConnected()) {
        sendStoredData();
        dataSent = true;
      }
      break;

    case DESCENDING:
      // Use target pitch for descent
      pitchController.setTargetPitch(targetPitch);
      pitchController.update();
      
      if (currentDepth >= diveDepth) {  // Use received diveDepth
        currentState = ASCENDING;
        ballast.setPosition(_ballastCLOSED);
        Serial.println("Reached target depth - beginning ascent");
        break;
      }
      
      ballast.setPosition(_ballastOPEN); // Keep open during descent
      break;

    case ASCENDING:
      // Maintain target pitch during ascent
      pitchController.setTargetPitch(targetPitch);
      pitchController.update();
      
      if (currentDepth <= _surfaceThreshold) {
        currentState = SURFACE_COMNS;
        dataSent = false;
        wifiComms.begin();
        Serial.println("Surfaced - ready for new commands");
        break;
      }
      break;

    case EMERGENCY:
      emergencyProcedure();
      break;
  }
}
//---------------------------------------------------Implementation-------------------------------------------

//-------------------------------------------------Surface Operations-----------------------------------------

void checkSurface() {
  depth = depthSensor.readDepthCm() / 100.0f;

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

// void onSurface() {
//     lastSurfaceTime = millis();
//     currentState = SURFACE_COMNS;
//     enableCommunications();
//     uploadData();
//     checkForCommands();
// }

void handleSurfaceOperations() {
  // Ensure SD card is available
  if (!sdCard.isAvailable()) {
    Serial.println("SD card not available for data transmission");
    return;
  }

  // Send all stored data line by line
  String dataLine;
  while ((dataLine = sdCard.readNextLine()).length() > 0) {
  wifiComms.sendData(dataLine);
    delay(10); // Small delay between transmissions
  }

  // Clear data file after successful transmission
  if (SD.exists(sdCard.getFileName())) {
    SD.remove(sdCard.getFileName());
    Serial.println("Cleared data file after transmission");
  }

  // Process incoming commands
  String cmd = wifiComms.receiveCommand();
  if (cmd.length() > 0) {
    processCommand(cmd);
  }
}

void sendStoredData() {
  String data;
  while ((data = sdCard.readNextLine()).length() > 0) {
    wifiComms.sendData(data);
    delay(10);  // Prevent flooding
  }
  //sdCard.clearDataFile();
}

//-------------------------------------------------Processing Data-----------------------------------------

void parseDiveParameters(String cmd) {
  int pitchIndex = cmd.indexOf("PITCH:");
  int depthIndex = cmd.indexOf("DEPTH:");
  
  if (pitchIndex != -1) {
    targetPitch = cmd.substring(pitchIndex + 6, cmd.indexOf(':', pitchIndex + 6)).toFloat();
    targetPitch = constrain(targetPitch, _minPitch, _maxPitch);
  }
  
  if (depthIndex != -1) {
    diveDepth = cmd.substring(depthIndex + 6).toFloat(); // Get depth from command
    diveDepth = constrain(diveDepth, _surfaceThreshold + 0.05f, _maxDepth); // Apply constraints
  }

  Serial.print("Dive params - Pitch: ");
  Serial.print(targetPitch);
  Serial.print("°, Depth: ");
  Serial.println(diveDepth);
}

void processCommand(String cmd) {
  if (cmd.startsWith("BEGIN_DIVE")) {
    parseDiveParameters(cmd);
    currentState = DESCENDING;
    dataSent = false;
    wifiComms.end();
    
    Serial.print("Beginning dive with Pitch:");
    Serial.print(targetPitch);
    Serial.print("° to Depth:");
    Serial.print(diveDepth);
    Serial.println("m");
  }
}

//----------------------------------------------------Emergency--------------------------------------------

void emergencyProcedure() {
  //imediately surface
  ballast.setPosition(_ballastCLOSED);
  pitchController.setTargetPitch(0);
  pitchController.update();
  
  //attempt communications
  wifiComms.begin();
  
  while (true) {
    updateSensorData();
    if (depthSensor.readDepthCm()/100.0f <= _surfaceThreshold) {
      currentState = SURFACE_COMNS;
      dataSent = false;
      break;
    }
    delay(100);
  }
}

#endif
