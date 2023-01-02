#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

// Server Functions

void readSettings() { //transfering ESP data to the Webpage
  String value = readFile("/data.json");
  if(value == ""){value = "ERROR: Lesen der Daten fehlgeschlagen.";}
  Serial.println("Received:" + value);
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void sendSettings() { //transfering start/endHour to the Webpage
  String value;
  value = String(startHour) + "," + String(endHour);
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void notFound() {
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}

void setStartHour() {
  String hour = server.arg("value");
  Serial.println("Setting startHour = " + hour);
  startHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Startzeitpunkt übertragen.");
}

void setEndHour() {
  String hour = server.arg("value");
  Serial.println("Setting endHour = " + hour);
  endHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Endzeitpunkt übertragen.");
}

void receiveSettings() {
  String jsonText = server.arg("value");
  String response = "";
  Serial.println("Receiving settings in JSON format: " + jsonText);
  if (writeFile("/data.json", jsonText.c_str())) {
    response = "Übertragen der Daten war erfolgreich!";
  } else {
    response = "ERROR: Übertragen der Daten fehlgeschlagen!";
  }
  server.send(200, "text/plane", response);
}

void closeSettings() {
  Serial.println("Closing Settings.");
  STATE_NEXT = STATE_INIT;
}

void fireworks() {
  Serial.println("Fireworks...");
  STATE_FOLLOWING = STATE_CONFIGURE;
  STATE_NEXT = STATE_SHOW;
  server.send(200, "text/plane", "Feuerwerk!"); //should always respond to prevent resend (10x)
}

void deleteSettings() {
  Serial.println("Delete Settings.");
  String response = "";
  if(deleteFile("/data.json")) {
    response = "Löschen der Daten war erfolgreich!";
  } else {
    response = "ERROR: Löschen der Daten fehlgeschlagen!";
  }
  server.send(200, "text/plane", response);
}

void startWebServer() {
  Serial.println("Starting WebServer...");
  server.on("/", handleRoot);
  server.on("/set_start", setStartHour);
  server.on("/set_end", setEndHour);
  server.on("/request_settings", sendSettings);
  server.on("/send_settings", receiveSettings);  //webpage => ESP name
  server.on("/read_settings", readSettings);
  server.on("/delete_settings", deleteSettings);
  server.on("/close", closeSettings);
  server.on("/fireworks", fireworks);  
  server.onNotFound(notFound);
  server.begin();
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(", hostName: http://");
  Serial.println(WiFi.getHostname());
}

void stopWebServer() {
  Serial.println("Stopping WebServer...");
  server.stop();
}
