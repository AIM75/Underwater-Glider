#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDLogger {
public:
  SDLogger(uint8_t csPin = 5);
  bool begin();
  bool logData(const String& dataLine);  // Changed from writeData to logData
  String readNextLine();                 // Changed from readNewData
  bool isAvailable() const {
    return _sdAvailable;
  }
  const String& getFileName() const {
    return _filename;
  }
  bool clearDataFile();

private:
  uint8_t _csPin;
  bool _sdAvailable;
  File _dataFile;
  const String _filename = "/glider_data.csv";

  bool openFile();
  void closeFile();
};

#endif
