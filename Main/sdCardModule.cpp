#include "sdCardModule.h"

SDLogger::SDLogger(uint8_t csPin) :
    _csPin(csPin),
    _sdAvailable(false)
{
}

bool SDLogger::begin() {
    if (!SD.begin(_csPin)) {
        Serial.println("SD card initialization failed!");
        _sdAvailable = false;
        return false;
    }
    
    Serial.println("SD card initialized successfully.");
    _sdAvailable = true;
    
    // Create header if file doesn't exist
    if (!SD.exists(_filename)) {
        openFile();
        _dataFile.println("Timestamp,Depth,Pitch,Roll,BallastPos");
        closeFile();
    }
    
    return true;
}

bool SDLogger::logData(const String &dataLine) {
    if (!_sdAvailable || !openFile()) return false;
    
    _dataFile.println(dataLine);
    closeFile();
    return true;
}

String SDLogger::readNextLine() {
    if (!_sdAvailable || !openFile()) return "";
    
    String line = _dataFile.readStringUntil('\n');
    line.trim();
    
    // If we reached end of file, start from beginning
    if (!_dataFile.available()) {
        _dataFile.seek(0);
        // Skip header
        _dataFile.readStringUntil('\n');
    }
    
    closeFile();
    return line;
}

bool SDLogger::openFile() {
    _dataFile = SD.open(_filename, FILE_APPEND);
    if (!_dataFile) {
        Serial.println("Error opening data file!");
        return false;
    }
    return true;
}

void SDLogger::closeFile() {
    if (_dataFile) {
        _dataFile.close();
    }
}
