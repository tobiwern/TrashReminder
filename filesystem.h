#include <memory>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
//https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
//https://makesmart.net/arduino-ide-arbeiten-mit-json-objekten-fur-einsteiger/
//https://arduinojson.org/v6/doc/deserialization/

#define JSON_MEMORY 1024 * 40

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
  showFSInfo();
  size_t size = file.size();
  char buf[JSON_MEMORY];
  file.readBytes(buf, size);
  file.close();
  endLittleFS();
  return (String(buf));
}

void readFile1(const char* fileName) {
  Serial.printf("INFO: Reading file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(fileName) + " for reading!");
    showFSInfo();
    return;
  }
  size_t size = file.size();
  if (size > JSON_MEMORY) {
    Serial.println("File size is too large " + String(size) + ">" + String(JSON_MEMORY));
    return;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);
  StaticJsonDocument<JSON_MEMORY> doc;
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  /*
  JsonObject obj = doc.as<JsonObject>(); //the root of a JSON object is always a DICT

  for (JsonPair p : obj) {
    p.key() // is a JsonString
    p.value() // is a JsonVariant
  }
*/
  // get colors ////////////////////////////////
  JsonVariant colorsJson = doc["colors"];

  JsonArray array = colorsJson.as<JsonArray>();
  for (JsonVariant v : array) {
    String colorText = v.as<String>();
    unsigned long int color = strtoul(colorText.c_str(), NULL, 16); //conversion from HEX String => HEX number
    Serial.println("Color: " + colorText + ", value = " + color);
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

boolean deleteFile(const char* fileName) {
  if (!startLittleFS()) { return (false); }
  Serial.printf("Deleting file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("WARNING: There is no file " + String(fileName) + ". Nothing to delete!");
    showFSInfo();
    return (true);
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
