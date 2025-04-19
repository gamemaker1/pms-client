#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// entry, exit, slots
#define MODE "entry"
// #define MODE "exit"

void setup() {
  Serial.begin(9600);

  setupPins(); connectToWiFi();
  if (MODE == "exit") setupServer();
}

void loop() {
  if (MODE == "entry") handleEntry();
  if (MODE == "exit") handleExit();

  handleLighting();
  handleParking();

  delay(1000);
}
