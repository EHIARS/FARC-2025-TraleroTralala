#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "conca";
const char* password = "12345678";

// Start HTTP server on port 80
WebServer server(80);

// === Parameters exposed via API ===
extern float speedScale, maxSpeedScale, rampUpStep, rampDownStep, rampDownFastStep, sinr, cosr;
extern unsigned long rampInterval;
extern float m1, m2, m3;


void handleGetConfig() {
  DynamicJsonDocument doc(512);
  doc["speedScale"] = speedScale;
  doc["maxSpeedScale"] = maxSpeedScale;
  doc["rampUpStep"] = rampUpStep;
  doc["rampDownStep"] = rampDownStep;
  doc["rampDownFastStep"] = rampDownFastStep;
  doc["rampInterval"] = rampInterval;
  doc["sinr"] = sinr;
  doc["cosr"] = cosr;
  doc["m1"] = m1;
  doc["m2"] = m2;
  doc["m3"] = m3;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}
void handlePostConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing JSON body");
    return;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("speedScale")) speedScale = doc["speedScale"];
  if (doc.containsKey("maxSpeedScale")) maxSpeedScale = doc["maxSpeedScale"];
  if (doc.containsKey("rampUpStep")) rampUpStep = doc["rampUpStep"];
  if (doc.containsKey("rampDownStep")) rampDownStep = doc["rampDownStep"];
  if (doc.containsKey("rampDownFastStep")) rampDownFastStep = doc["rampDownFastStep"];
  if (doc.containsKey("rampInterval")) rampInterval = doc["rampInterval"];
  if (doc.containsKey("sinr")) sinr = doc["sinr"];
  if (doc.containsKey("cosr")) cosr = doc["cosr"];

  server.send(200, "application/json", "{\"status\":\"updated\"}");
}

void setupAdjustmentServer() {
  server.on("/config", HTTP_GET, handleGetConfig);
  server.on("/config", HTTP_POST, handlePostConfig);
  server.begin();
  Serial.println("🌐 Server started at /config");
}

void handleAdjustment() {
  server.handleClient();
}