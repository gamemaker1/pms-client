#define ENTRY_IR_SENSOR 34
#define LED_LIGHT_PIN   23

#define CAR_ENTRY_THRESHOLD 500
#define PARK_TIME_THRESHOLD 4000
#define EXIT_TIME_THRESHOLD 7000

void connectToWiFi() {
  Serial.print("[wifi] connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); Serial.print(".");
  }

  Serial.println("\n[wifi] connected");
}

WiFiServer server(5252);
void setupServer() {
  Serial.println("[exit] starting server...");
  server.begin();
  IPAddress ip = WiFi.localIP();

  Serial.print("[exit] listening on http://");
  Serial.print(ip); Serial.println(":5252");
}

JsonDocument postRequest(String endpoint, String request) {
  HTTPClient http; http.begin(serverUrl + endpoint);
  http.setTimeout(5000);
  http.addHeader("content-type", "application/json");
  http.addHeader("authorization", "device " + secret);

  int response = http.POST(request);
  Serial.print("[http] made post request, got response ");
  Serial.println(response);
  JsonDocument doc; String payload = "{\"error\": {\"code\": \"internal-error\"}}";
  if (response <= 0) {
    Serial.println("[http] error making post request");
    Serial.println(http.errorToString(response).c_str());
  } else payload = response == 204 ? "{}" : http.getString();

  http.end();
  deserializeJson(doc, payload);
  return doc;
}

void handleLighting() {
  bool empty = true;
  for (auto slot : slots) {
    if (slot.parked || slot.detected > 0) {
      // Serial.print("[lights] activity in ");
      // Serial.println(slot.location);
      empty = false; break;
    }
  }

  digitalWrite(LED_LIGHT_PIN, !empty);
}

bool handlingEntry = false;
void handleEntry() {
  int dist = analogRead(ENTRY_IR_SENSOR);
  // Serial.print("[entry] sensor reading: "); Serial.println(dist);

  if (dist < CAR_ENTRY_THRESHOLD && !handlingEntry) {
    Serial.println("[entry] detected car");

    // TODO: handle bikes as well.
    JsonDocument payload = postRequest("/sessions", "{\"vehicle\": \"lmv\"}");
    String otp = payload["data"]["symbol"]; if (otp == "null") return;

    Serial.print("[entry] got otp: ");
    Serial.println(otp); writeOtp(otp);

    handlingEntry = true; return;
  }

  if (handlingEntry) {
    Serial.println("[entry] opening gate for car");
    openGate(); delay(5000);

    Serial.println("[entry] closing gate for car");
    closeGate(); writeOtp("          ");

    handlingEntry = false; return;
  }
}

void handleParking() {
  for (int i = 0; i < SLOT_COUNT; i++) {
    Slot* slot = &slots[i]; int dist = getDistance(slot->trig, slot->echo);
    // Serial.print("[parking] sensor reading: "); Serial.print(slot->location);
    // Serial.print(" - "); Serial.println(dist);

    if (dist <= slot->threshold && !slot->parked) {
      if (slot->detected <= 0) {
        Serial.print("[parking] car park attempt for slot ");
        Serial.println(slot->location);

        digitalWrite(slot->gled, HIGH);
        digitalWrite(slot->rled, LOW);

        slot->detected = millis(); continue;
      }

      if (millis() - slot->detected >= PARK_TIME_THRESHOLD) {
        Serial.print("[parking] parking car in slot ");
        Serial.println(slot->location);

        JsonDocument response = postRequest(
          "/slots/" + slot->location + "/park", "{}"
        );

        if (response["error"]) {
          Serial.print("[parking] failed to park due to error: ");
          Serial.println(response["error"]["code"].as<String>());
          slot->parked = false; slot->detected = 0;
          blinkLed(slot->rled, 3); digitalWrite(slot->gled, HIGH);
        } else {
          Serial.print("[parking] started meter for car in slot ");
          Serial.println(slot->location);
          slot->parked = true; slot->detected = 0;
          digitalWrite(slot->gled, LOW); digitalWrite(slot->rled, HIGH);
        }
      }
    }

    if (dist > slot->threshold) {
      if (slot->parked) {
        if (slot->detected <= 0) {
          Serial.print("[parking] car leave attempt for slot ");
          Serial.println(slot->location);

          digitalWrite(slot->gled, LOW);
          digitalWrite(slot->rled, HIGH);

          slot->detected = millis(); continue;
        }

        if (millis() - slot->detected >= EXIT_TIME_THRESHOLD) {
          Serial.print("[parking] leaving slot "); Serial.println(slot->location);

          JsonDocument response = postRequest(
            "/slots/" + slot->location + "/exit", "{}"
          );

          if (response["error"]) {
            Serial.print("[parking] failed to leave due to error: ");
            Serial.println(response["error"]["code"].as<String>());

            if (response["error"]["code"] == "not-parked") {
              slot->parked = false; slot->detected = 0;
              digitalWrite(slot->rled, LOW);
              digitalWrite(slot->gled, HIGH);
            } else {
              blinkLed(slot->rled, 3); continue;
            }
          } else {
            digitalWrite(slot->rled, LOW);
            digitalWrite(slot->gled, HIGH);

            slot->parked = false; slot->detected = 0;
          }
        }
      } else if (slot->detected > 0) {
        Serial.print("[parking] cancelled attempt for ");
        Serial.println(slot->location);

        digitalWrite(slot->rled, LOW);
        digitalWrite(slot->gled, HIGH);

        slot->parked = false; slot->detected = 0;
      }
    }
  }
}

void handleExit() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("[exit] received request to open gate");

  bool blank = true;
  while (client.connected()) {
    if (!client.available()) continue;

    char c = client.read();
    if (c == '\n' && blank) {
      client.println("HTTP/1.1 204 No Content");
      client.println("Connection: close");
      client.println(); client.println();
      break;
    }

    if (c == '\n') blank = true;
    else if (c != '\r') blank = false;
  }

  delay(1); client.stop();

  Serial.println("[exit] opening gate for car");
  openGate(); delay(5000);

  Serial.println("[exit] closing gate for car");
  closeGate();
}
