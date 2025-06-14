#include "DataLogger.h"

DataLogger::DataLogger(const char* filename, size_t maxFileSize) 
    : _filename(filename), _maxFileSize(maxFileSize) {}

bool DataLogger::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }
    return true;
}

bool DataLogger::logData(const String& data) {
    if (!_checkFileSize()) {
        Serial.println("File size check failed");
        return false;
    }

    File file = SPIFFS.open(_filename, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }

    if (file.println(data)) {
        file.close();
        return true;
    }
    
    file.close();
    return false;
}

String DataLogger::readData() {
    File file = SPIFFS.open(_filename);
    if (!file || file.isDirectory()) {
        return String();
    }

    String content;
    while (file.available()) {
        content += file.readString();
    }
    file.close();
    return content;
}

bool DataLogger::clearData() {
    return SPIFFS.remove(_filename);
}

size_t DataLogger::getDataSize() {
    File file = SPIFFS.open(_filename);
    if (!file) return 0;
    
    size_t size = file.size();
    file.close();
    return size;
}

bool DataLogger::isAvailable() {
    return SPIFFS.exists(_filename);
}

bool DataLogger::_checkFileSize() {
    size_t currentSize = getDataSize();
    if (currentSize >= _maxFileSize) {
        Serial.println("Maximum file size reached, clearing data");
        return clearData();
    }
    return true;
}