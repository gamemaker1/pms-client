#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void setup() {
	Serial.begin(9600);
	setupPins();
	connectToWiFi();
}

void loop() {
	handleEntry();
	handleParking();
	delay(1000);
}
