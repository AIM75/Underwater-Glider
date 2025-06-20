#ifndef APPLICATION_H
#define APPLICATION_H


// Include section
#include "Orientation.h"
#include "DepthModule.h"
#include "sdCardModule.h"
#include "DataLoggerESP.h"
#include "CommunicationModule.h"
#include "Ballast.h"
#include "PitchController.h"



//-------------------------------------------------------Configurations------------------------------------//

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
#define _ballastOPEN 30      // in deg
#define _ballastCLOSED 160   // in deg


PitchConfig config = {
  // Pin configuration
  .step_pin = _stepPIN,
  .dir_pin = _dirPIN,
  .sleep_pin = _slpPIN,
  .limit_switch_min_pin = _limSW2PIN,
  .limit_switch_max_pin = _limSW1PIN,

  // Mechanical properties
  .steps_per_rev = 200,
  .lead_screw_pitch = 8.0f,
  .max_travel = 65.0f,

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
DataLogger dataLogger("/glider_data.log");
WiFiComms wifiComms(ssid, password, local_ip, gateway, subnet, wifi_port);
PitchController pitchController(config);
Ballast myServo(_servoPIN);
Servo ser;

//------------------------------------------------------------Variables-------------------------

enum GliderState { SURFACE_COMNS,
                   DESCENDING,
                   ASCENDING,
                   EMERGENCY };
GliderState currentState = SURFACE_COMNS;

float targetPitchU = _defaultTHEAT;   //  pitch angle
float targetPitchD = -_defaultTHEAT;  // DefDefaultault pitch angle
float maxDepth = _maxDepth;           // Default max depth (cm)

float depth;        // in cm
float pitch, roll;  // in deg
float ballastPos = myServo.getPosition();
bool isMassMoved = false;

// ------------------------------------------------------Functions Declarations-----------------------------------------
bool initializeModules();
void DataLogging();
void checkSurface();
void handleSurfaceOperations();
void parseDiveParameters(String cmd);
void processCommand(String cmd);
void runStateMachine();
void emergencyProcedure();

//----------------------------------------------Functions Definitions----------------------------------------

//-------------------------------------------Initialization and Updating-------------------------------------

bool initializeModules() {
  // myServo.begin(_ballastCLOSED);
  ser.attach(32);
  ser.write(160);
  depthSensor.begin();
  wifiComms.begin();
  orientation.begin();
  sdCard.begin();
  dataLogger.begin();
  pitchController.begin();

  return true;
}


void DataLogging() {

  depth = depthSensor.readDepthCm();
  pitch = -101;  // just a pre defined No. to figure out if the orientaion is not updated due to fault in measurements
  roll = -101;   // just a pre defined No. to figure out if the orientaion is not updated due to fault in measurements
  if (orientation.update()) {
    pitch = orientation.getPitch();
    roll = orientation.getRoll();
  }
  ballastPos = myServo.getPosition();
  String data = "Data:" + String(pitch, 1) + "," + String(depth, 2) + "::Servo:" + String(ballastPos);

  if (sdCard.begin() == 0)
    sdCard.logData(data);
  else
    dataLogger.logData(data);
  Serial.println(data);
  delay(50);
}

//---------------------------------------------------State Machine-------------------------------------------

void runStateMachine() {
  depth = depthSensor.readDepthCm();

  switch (currentState) {
    Serial.println("::Surface::");
    case SURFACE_COMNS:
      while (currentState == SURFACE_COMNS) {  // to prevent data logging at surface
        checkSurface();
      }
      break;

    case DESCENDING:
      Serial.println("Descending");
      if (isMassMoved == false) {
        pitchController.setDivePhase(PitchController::DivePhase::DESCENDING);
        pitchController.setTargetPitch(targetPitchD);
        while (pitchController.getStepsToGo() != 0) {
          pitchController.update();
        }
      } else {
        // myServo.setPosition(_ballastOPEN);
        ser.write(30);
      }
        isMassMoved = true;




      if (depth > maxDepth) {
        currentState = ASCENDING;
        // myServo.setPosition(_ballastCLOSED);
        isMassMoved = false;
        Serial.println("Reached target depth - beginning ascent");
        break;
      }
      break;

    case ASCENDING:
      Serial.println("Ascending");
      // myServo.setPosition(_ballastCLOSED);
      ser.write(160);
      if (isMassMoved == false) {
        pitchController.setDivePhase(PitchController::DivePhase::ASCENDING);
        pitchController.setTargetPitch(targetPitchU);

        while (pitchController.getStepsToGo() != 0) {
          pitchController.update();
        }
        isMassMoved = true;
      }
      if (depth < _surfaceDepth) {
        currentState = SURFACE_COMNS;
        isMassMoved = false;
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

void checkSurface() {  // insures that clinet is connected and opened the server before surface operation starts
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

  }

  else if (cmd.startsWith("CHGPIT")) {  // If you need not to power down WIFI
    parseDiveParameters(cmd);
    currentState = DESCENDING;
  }

  else if (cmd.startsWith("GET_DATA")) {
    Serial.println("Data");
    if (sdCard.isAvailable()) {
      String data;
      while ((data = sdCard.readNextLine()).length() > 0) {
        wifiComms.sendData(data);
        delay(10);  // Prevent flooding
      }
    } else {
      String loggedData = dataLogger.readData();
      if (loggedData.length() > 0) {
        wifiComms.sendData(loggedData);
        Serial.println("Sent all logged data to client");
      }
    }
  }


else if (cmd.startsWith("CLEAR_DATA")) {
  if (sdCard.isAvailable()) {
    sdCard.clearDataFile();
    Serial.println("Cleared data file after transmission");
  } else {
    if (dataLogger.isAvailable()) {
      dataLogger.clearData();
      Serial.println("Cleared data file after transmission");
    }
  }
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
    maxDepth = cmd.substring(depthIndex + 6).toFloat();                     // Get depth from command
  }
}


void emergencyProcedure() {
}

#endif
