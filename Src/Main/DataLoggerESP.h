#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <SPIFFS.h>
#include <Arduino.h>

class DataLogger {
public:
    DataLogger(const char* filename = "/sensor_data.log", size_t maxFileSize = 1024 * 1024); // 1MB default
    
    bool begin();
    bool logData(const String& data);
    String readData();
    bool clearData();
    size_t getDataSize();
    bool isAvailable();

private:
    const char* _filename;
    const size_t _maxFileSize;
    
    bool _checkFileSize();
};

#endif