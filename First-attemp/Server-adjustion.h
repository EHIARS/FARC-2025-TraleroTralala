#ifndef SERVER_ADJUSTION_H
#define SERVER_ADJUSTION_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "motorServoPs2.h"
// WiFi Configuration - MAKE SURE TO UPDATE THESE!
const char* ssid = "conca";       // Change to your network
const char* password = "12345678"; // Change to your password
const char* apSSID = "ESP32_Config";
const char* apPassword = "password123";

WebServer server(80);

// Shared variables
extern float speedScale, maxSpeedScale, rampUpStep, rampDownStep, rampDownFastStep, sinr, cosr;
extern unsigned long rampInterval;
extern float m1, m2, m3;

void setupWiFi() {
  Serial.println("\nConnecting to WiFi...");
  WiFi.mode(WIFI_AP_STA);  // Enable both STA and AP modes simultaneously
  
  // 1. Try connecting to your WiFi
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(50);
    Serial.print(".");
    attempts++;
  }

  // 2. Start AP mode regardless of STA connection status
  Serial.println("\nStarting AP Mode");
  WiFi.softAP(apSSID, apPassword);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // 3. Start mDNS responder
  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up mDNS!");
  } else {
    Serial.println("mDNS responder started (esp32.local)");
  }
}

void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }
}

void handleGetConfig() {
  enableCORS();
  
  DynamicJsonDocument doc(512);
  doc["speedScale"] = speedScale;
  doc["maxSpeedScale"] = maxSpeedScale;
  doc["rampUpStep"] = rampUpStep;
  doc["rampDownStep"] = rampDownStep;
  doc["rampDownFastStep"] = rampDownFastStep;
  doc["rampInterval"] = rampInterval;
  doc["sinr"] = sinr;
  doc["cosr"] = cosr;
  doc["m1"] = pwm.getPWM(motor1[0])-pwm.getPWM(motor1[1]);
  doc["m2"] = pwm.getPWM(motor2[0])-pwm.getPWM(motor2[1]);
  doc["m3"] = pwm.getPWM(motor3[0])-pwm.getPWM(motor3[1]);
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handlePostConfig() {
  enableCORS();
  
  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
    return;
  }

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

  // Update parameters
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
  server.on("/config", HTTP_OPTIONS, []() {
    enableCORS();
    server.send(204);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

void handleAdjustment() {
  server.handleClient();
}

#endif