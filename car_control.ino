#include <Servo.h>

Servo steeringServo;
const int motorPin1 = 4;  // L298N IN1
const int motorPin2 = 5;  // L298N IN2
const int motorPin3 = 6;  // L298N IN3
const int motorPin4 = 7;  // L298N IN4
const int servoPin = 9;   // Servo pin
const int rxPin = 10;     // SIM800L TXD
const int txPin = 11;     // SIM800L RXD

String command = "";

void setup() {
  Serial.begin(9600);
  steeringServo.attach(servoPin);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "FORWARD") {
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
      digitalWrite(motorPin3, HIGH);
      digitalWrite(motorPin4, LOW);
      steeringServo.write(90); // Прямо
    }
    else if (command == "BACKWARD") {
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, HIGH);
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin4, HIGH);
      steeringServo.write(90);
    }
    else if (command == "LEFT") {
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin4, LOW);
      steeringServo.write(0); // Налево
    }
    else if (command == "RIGHT") {
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, LOW);
      digitalWrite(motorPin3, HIGH);
      digitalWrite(motorPin4, LOW);
      steeringServo.write(180); // Направо
    }
    else if (command == "STOP") {
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, LOW);
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin4, LOW);
      steeringServo.write(90);
    }
  }
}