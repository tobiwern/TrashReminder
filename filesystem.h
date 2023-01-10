#include <memory>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
//https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
//https://makesmart.net/arduino-ide-arbeiten-mit-json-objekten-fur-einsteiger/
//https://arduinojson.org/v6/doc/deserialization/



boolean startLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("ERROR: Failed to mount LittleFS.");
    return (false);
  }
  Serial.println("INFO: Successfully mounted LittleFS.");
  return (true);
}

void endLittleFS() {
  LittleFS.end();
  Serial.println("INFO: Successfully un-mounted LittleFS.");
}

boolean writeFile(const char* fileName, const char* message) {
  if (!startLittleFS()) { return (false); }
  File file = LittleFS.open(fileName, "w");
  if (!file) {
    Serial.println("ERROR: Failed to open file " + String(fileName) + " for writing!");
    return (false);
  }
  if (file.print(message)) {
    Serial.println("INFO: Successfully wrote file " + String(fileName) + "!");
  } else {
    Serial.println("ERROR: Failed to write file " + String(fileName) + "!");
    return (false);
  }
  size_t size = file.size();
  Serial.println("INFO: File size is " + String(size) + ".");
  file.close();
  endLittleFS();
  return (true);
}

boolean listDir(const char* dirname) {
  if (!startLittleFS()) { return (false); }
  Serial.printf("Listing directory: %s\n", dirname);
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    File file = root.openFile("r");
    Serial.print(" FILE: ");
    Serial.print(root.fileName());
    Serial.print(" SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm* tmstruct = localtime(&cr);
    Serial.printf(", CREATION: %d-%02d-%02d %02d:%02d:%02d, ", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf(", LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
  endLittleFS();
  return (true);
}

boolean showFSInfo() {
  if (!startLittleFS()) { return (false); }
  FSInfo info;
  LittleFS.info(info);
  Serial.println("totalBytes: " + String(info.totalBytes));
  Serial.println("usedBytes: " + String(info.usedBytes));
  Serial.println("blockSize: " + String(info.blockSize));
  Serial.println("pageSize: " + String(info.pageSize));
  Serial.println("maxOpenFiles: " + String(info.maxOpenFiles));
  Serial.println("maxPathLength: " + String(info.maxPathLength));
  listDir("/");
  endLittleFS();
  return (true);
}

String readFile(const char* fileName) {
  //File System
  if (!startLittleFS()) { return (""); }
  Serial.printf("INFO: Reading file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(fileName) + " for reading!");
    return ("");
  }
  String jsonText = file.readString();
  file.close();
  endLittleFS();
  return (jsonText);
}

boolean deleteFile(const char* fileName) {
  if (!startLittleFS()) { return (false); }
  Serial.printf("Deleting file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("WARNING: There is no file " + String(fileName) + ". Nothing to delete!");
    showFSInfo();
    return (false);
  }
  if (LittleFS.remove(fileName)) {
    Serial.println("INFO: Sucessfully deleted file " + String(fileName) + ".");
  } else {
    Serial.println("ERROR: Failed to delete file " + String(fileName) + ".");
    return (false);
  }
  endLittleFS();
  return (true);
}

boolean renameFile(const char* fileName1, const char* fileName2) {
  if (!startLittleFS()) { return (false); }
  Serial.printf("Renaming file %s to %s\n", fileName1, fileName2);
  if (LittleFS.rename(fileName1, fileName2)) {
    Serial.println("INFO: File renamed.");
  } else {
    Serial.println("ERROR: Rename failed.");
    return (false);
  }
  endLittleFS();
  return (true);
}
