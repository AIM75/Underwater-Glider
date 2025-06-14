// Stepper motor pins (adjust for your ESP32 board)
#define STEP_PIN 33
#define DIR_PIN 25

const uint16_t stepsPerRevolution = 200*32;    // 1.8° stepper

void setup() {
pinMode(STEP_PIN, OUTPUT);
dacDisable(DIR_PIN);
pinMode(DIR_PIN, OUTPUT);
pinMode(26, INPUT);
pinMode(27, INPUT);
pinMode(13, OUTPUT);

Serial.begin(115200);

digitalWrite(DIR_PIN, LOW);
}

void loop() {
digitalWrite(13, HIGH);  
for (int i = 0; i < stepsPerRevolution; i++) {
digitalWrite(STEP_PIN, HIGH);
delayMicroseconds(600); // Adjust this to control motor speed
digitalWrite(STEP_PIN, LOW);
delayMicroseconds(600);
Serial.println(digitalRead(26));
Serial.println(digitalRead(27));
Serial.println("---------------------------------");
}
delay(1000); // Wait for 1 second before changing direction
digitalWrite(13, LOW);
digitalWrite(DIR_PIN, 1); // Change direction to counter-clockwise

for (int i = 0; i < stepsPerRevolution; i++) {
digitalWrite(STEP_PIN, HIGH);
delayMicroseconds(600);
digitalWrite(STEP_PIN, LOW);
delayMicroseconds(600);
Serial.println(digitalRead(26));
Serial.println(digitalRead(27));
Serial.println("---------------------------------");
}
delay(1000); // Wait for 1 second before changing direction

digitalWrite(DIR_PIN, 0); // Change direction to clockwise
}
