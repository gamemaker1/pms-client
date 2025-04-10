#define SERVO_MOTOR_PIN 13
#define ENTRY_IR_SENSOR 34

#define SPEED_OF_SOUND 0.0343
#define SENSOR_TIMEOUT 20000
#define VALUE_INFINITY 1000000000

Servo motor;
void setupPins() {
	motor.attach(SERVO_MOTOR_PIN);
	pinMode(ENTRY_IR_SENSOR, INPUT);
	
	for (auto slot : slots) {
		pinMode(slot.trig, OUTPUT);
		pinMode(slot.echo, INPUT);
		pinMode(slot.gled, OUTPUT);
		pinMode(slot.rled, OUTPUT);

		digitalWrite(slot.gled, HIGH);
		digitalWrite(slot.rled, LOW);
	}
}

void openGate() {
	for (int pos = 0; pos <= 90; pos++) {
		motor.write(pos); delay(10);
	}
}

void closeGate() {
	for (int pos = 90; pos >= 0; pos--) {
		motor.write(pos); delay(10);
	}
}

int getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, SENSOR_TIMEOUT);
  if (duration == 0) return VALUE_INFINITY;
  return (duration * SPEED_OF_SOUND) / 2;
}

void blinkLed(int pin, int count) {
	for (int i = 0; i < count; i++) {
		digitalWrite(pin, HIGH); delay(100);
		digitalWrite(pin, LOW); delay(100);
	}
}
