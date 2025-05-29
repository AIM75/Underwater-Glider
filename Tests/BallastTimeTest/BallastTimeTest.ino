#include <Servo.h>
Servo myservo;  
unsigned long lastTime = 0;
int time = 0 ;
void Beeping1();
void Beeping2();

void setup() {
    Serial.begin(9600);
    myservo.attach(9);
    pinMode(2,OUTPUT);
    digitalWrite(2,LOW);
    myservo.write(140);
    
    // Optional: Perform calibration if known pressure is available
    // pressureSensor.calibrate(0.0); // Calibrate at 0 pressure (atmospheric)
    lastTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    if(currentTime / 1000.0f > time + 60){
      Beeping1();
      delay(10000);
      myservo.write(60); 
      delay(7000);
      myservo.write(140); 
      Beeping2();
      time = millis() /1000.0f;
      delay(8000);
      myservo.write(140);
    }
    
    Serial.println((currentTime) / 1000.0f, 3);
}
void Beeping1(){
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
      delay(500);
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
      delay(500);
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
      delay(500);
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
}
void Beeping2(){
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
      delay(500);
      digitalWrite(2,HIGH);
      delay(500);
      digitalWrite(2,LOW);
}