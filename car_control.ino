#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial sim800(10, 11); // RX, TX
Servo steeringServo;

const int motorPin1 = 4;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 7;
const int servoPin = 9;

String command = "";

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  steeringServo.attach(servoPin);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  delay(3000); // ждем подключения SIM
  setupGPRS();
}

void loop() {
  if (sim800.available()) {
    String line = sim800.readStringUntil('\n');
    if (line.startsWith("GET")) {
      int cmdStart = line.indexOf("command=") + 8;
      String cmd = line.substring(cmdStart, line.indexOf(' ', cmdStart));
      cmd.trim();
      processCommand(cmd);
    }
  }
}

void setupGPRS() {
  sendAT("AT");
  sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendAT("AT+SAPBR=3,1,\"APN\",\"internet\""); // Замените на APN оператора
  sendAT("AT+SAPBR=1,1");
  sendAT("AT+SAPBR=2,1");
  sendAT("AT+HTTPINIT");
  sendAT("AT+HTTPPARA=\"CID\",1");
  sendAT("AT+HTTPPARA=\"URL\",\"http://YOUR_PUBLIC_IP:8000/get-command\""); // Замените на ваш EC2 IP
  sendAT("AT+HTTPACTION=0");
}

void sendAT(String cmd) {
  sim800.println(cmd);
  delay(1000);
  while (sim800.available()) {
    Serial.println(sim800.readString());
  }
}

void processCommand(String cmd) {
  if (cmd == "FORWARD") {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
    steeringServo.write(90);
  } else if (cmd == "BACKWARD") {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
    steeringServo.write(90);
  } else if (cmd == "LEFT") {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    steeringServo.write(0);
  } else if (cmd == "RIGHT") {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
    steeringServo.write(180);
  } else {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    steeringServo.write(90);
  }
}
