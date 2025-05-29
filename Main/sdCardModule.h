#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDLogger {
public:
    // Constructor with default pin configuration for ESP32 DOIT DevKit v1
    SDLogger(uint8_t csPin = 5, uint8_t clkPin = 18, uint8_t misoPin = 19, uint8_t mosiPin = 23);
    
    // Initialize SD card
    bool begin();
    
    // Write data to file
    bool writeData(const String &data);
    
    // Read new data since last read
    String readNewData();
    
    // Check if SD card is available
    bool isAvailable() const { return _sdAvailable; }
    
    // Generate sample data (for demonstration)
    String generateSampleData();

private:
    uint8_t _csPin;
    uint8_t _clkPin;
    uint8_t _misoPin;
    uint8_t _mosiPin;
    
    bool _sdAvailable;
    unsigned long _filePosition;
    const String _filename = "/data.txt";
    
    bool ensureFileExists();
};

#endif