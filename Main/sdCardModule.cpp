#include "sdCardModule.h"
#include <Arduino.h>

SDLogger::SDLogger(uint8_t csPin, uint8_t clkPin, uint8_t misoPin, uint8_t mosiPin) :
    _csPin(csPin),
    _clkPin(clkPin),
    _misoPin(misoPin),
    _mosiPin(mosiPin),
    _sdAvailable(false),
    _filePosition(0)
{
}

bool SDLogger::begin() {
    // Initialize SPI bus for SD card
    SPI.begin(_clkPin, _misoPin, _mosiPin, _csPin);
    
    Serial.println(F("Initializing SD card..."));

    if (!SD.begin(_csPin)) {
        Serial.println(F("SD card initialization failed!"));
        Serial.println(F("Please check:"));
        Serial.println(F("- SD card is inserted"));
        Serial.println(F("- SD card format (FAT32 recommended)"));
        Serial.println(F("- Wiring is correct"));
        _sdAvailable = false;
        return false;
    }

    Serial.println(F("SD card initialized successfully."));
    _sdAvailable = true;
    
    return ensureFileExists();
}

bool SDLogger::writeData(const String &data) {
    if (!_sdAvailable) return false;

    File dataFile = SD.open(_filename.c_str(), FILE_WRITE);
    if (!dataFile) {
        Serial.println(F("Error opening file for writing!"));
        return false;
    }

    int bytesWritten = dataFile.println(data);
    dataFile.close();

    if (bytesWritten <= 0) {
        Serial.println(F("Write failed!"));
        return false;
    }

    Serial.print(F("Data written: "));
    Serial.println(data);
    return true;
}

String SDLogger::readNewData() {
    if (!_sdAvailable) return "";

    String newData;
    File dataFile = SD.open(_filename.c_str());
    if (!dataFile) {
        Serial.println(F("Error opening file for reading!"));
        return "";
    }

    dataFile.seek(_filePosition);
    
    while (dataFile.available()) {
        String line = dataFile.readStringUntil(';');
        line.trim();
        if (line.length() > 0) {
            if (newData.length() > 0) newData += "\n";
            newData += line;
        }
    }
    
    _filePosition = dataFile.position();
    dataFile.close();

    return newData;
}

String SDLogger::generateSampleData() {
    float a = millis() / 1000.0 + 55;
    float b = millis() / 1000.0 + 34;
    float c = millis() / 1000.0 + 48;
    float d = millis() / 1000.0 + 11;
    float e = millis() / 1000.0 + 89;

    return String(a) + "," + String(b) + "," + String(c) + "," + String(d) + "," + String(e) + ";";
}

bool SDLogger::ensureFileExists() {
    if (!_sdAvailable) return false;

    if (!SD.exists(_filename.c_str())) {
        Serial.println(F("Creating data file..."));
        File dataFile = SD.open(_filename.c_str(), FILE_WRITE);
        if (dataFile) {
            dataFile.close();
            _filePosition = 0;
            Serial.println(F("File created successfully."));
            return true;
        } else {
            Serial.println(F("Error creating file!"));
            return false;
        }
    }
    return true;
}