#include <memory>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
//https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
//https://makesmart.net/arduino-ide-arbeiten-mit-json-objekten-fur-einsteiger/
//https://arduinojson.org/v6/doc/deserialization/

#define JSON_MEMORY 1024*10

void setupFS() {
  if (!LittleFS.begin()) {
    Serial.println("ERROR: Failed to mount LittleFS.");
  } else {
    Serial.println("INFO: Successfully mounted LittleFS.");
  }
}

void writeFile(const char* fileName, const char* message) {
  Serial.printf("INFO: Writing file: %s\n", fileName);
  File file = LittleFS.open(fileName, "w");
  if (!file) {
    Serial.println("ERROR: Failed to open file " + String(fileName) + " for writing!");
    return;
  }
  if (file.print(message)) {
    Serial.println("INFO: Successfully wrote file " + String(fileName) + "!");
  } else {
    Serial.println("ERROR: Failed to write file " + String(fileName) + "!");
    return;
  }
  size_t size = file.size();
  Serial.println("INFO: File size is " + String(size) + ".");
  file.close();
}

void listDir(const char* dirname) {
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
}

void showFSInfo(){
  FSInfo info;
  LittleFS.info(info);
  Serial.println("totalBytes: " + String(info.totalBytes));
  Serial.println("usedBytes: " + String(info.usedBytes));
  Serial.println("blockSize: " + String(info.blockSize));
  Serial.println("pageSize: " + String(info.pageSize));
  Serial.println("maxOpenFiles: " + String(info.maxOpenFiles));
  Serial.println("maxPathLength: " + String(info.maxPathLength));
  listDir("/");
}

//ToDo: mount and unmount for each operation
String readFile(const char* fileName) {
//  void readFile(const char* fileName) {
  Serial.printf("INFO: Reading file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(fileName) + " for reading!");
//    return("");
  }
  showFSInfo();
  size_t size = file.size();
  Serial.println("INFO: File size is " + String(size) + ".");
//  std::unique_ptr<char[]> buf(new char[size]);
  char buf[JSON_MEMORY*4];
  file.readBytes(buf, size);
//  Serial.println("Received: " + String(buf));
//  file.readBytes(buf.get(), size);

//  Serial.print("Read from file: ");
//  while (file.available()) {
//    Serial.write(file.read());
//  }
  

  file.close();
  return(String(buf));
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
    unsigned long int color = strtoul(colorText.c_str(), NULL, 16);
    Serial.println("Color: " + colorText + ", value = " + color);
  }

  Serial.println("atoi(0xFFFFFF): " + String(atoi("0xFFFFFF")) + ", String(0xFFFFFF): " + String(0xFFFFFF));

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void deleteFile(const char* fileName) {
  Serial.printf("Deleting file: %s\n", fileName);
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    Serial.println("WARNING: There is no file " + String(fileName) + ". Nothing to delete!");
    showFSInfo();
    return;
  }  
  if (LittleFS.remove(fileName)) {
    Serial.println("INFO: Sucessfully deleted file " + String(fileName) + ".");
  } else {
    Serial.println("ERROR: Failed to delete file " + String(fileName) + ".");
  }
}

void renameFile(const char* fileName1, const char* fileName2) {
  Serial.printf("Renaming file %s to %s\n", fileName1, fileName2);
  if (LittleFS.rename(fileName1, fileName2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}




