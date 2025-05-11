#include <Servo.h>
#include <SoftwareSerial.h>

// Пины для драйвера L298N
const int motor1Pin1 = 4;  // IN1
const int motor1Pin2 = 5;  // IN2
const int motor2Pin1 = 6;  // IN3
const int motor2Pin2 = 7;  // IN4

// Пин для сервопривода руля
const int servoPin = 9;

// Пины для SIM800L
const int sim800lTx = 10;
const int sim800lRx = 11;

// Создаем объекты
Servo steeringServo;
SoftwareSerial sim800l(sim800lRx, sim800lTx);

// Переменные для хранения команд
String currentCommand = "STOP";
int steeringAngle = 90;  // Центральное положение руля

void setup() {
  // Инициализация пинов драйвера
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  
  // Инициализация сервопривода
  steeringServo.attach(servoPin);
  steeringServo.write(steeringAngle);
  
  // Инициализация Serial для отладки
  Serial.begin(9600);
  
  // Инициализация SIM800L
  sim800l.begin(9600);
  
  // Остановка моторов при запуске
  stopMotors();
}

void loop() {
  // Проверяем наличие данных от SIM800L
  if (sim800l.available()) {
    String command = sim800l.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
  
  // Проверяем наличие данных от Serial для отладки
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
}

void processCommand(String command) {
  if (command == "FORWARD") {
    moveForward();
  } else if (command == "BACKWARD") {
    moveBackward();
  } else if (command == "LEFT") {
    turnLeft();
  } else if (command == "RIGHT") {
    turnRight();
  } else if (command == "STOP") {
    stopMotors();
  }
}

void moveForward() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void turnLeft() {
  steeringAngle = 45;  // Поворот руля влево
  steeringServo.write(steeringAngle);
  moveForward();
}

void turnRight() {
  steeringAngle = 135;  // Поворот руля вправо
  steeringServo.write(steeringAngle);
  moveForward();
}

void stopMotors() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  steeringAngle = 90;  // Возврат руля в центральное положение
  steeringServo.write(steeringAngle);
} 