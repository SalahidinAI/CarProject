#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial sim800(10, 11); // RX, TX
Servo steeringServo;

const int motorPin1 = 4;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 7;
const int servoPin = 9;

String lastCommand = "";
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 2000; // Интервал запросов 2 секунды

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  steeringServo.attach(servoPin);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  delay(3000); // Ждем подключения SIM
  setupGPRS();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Проверяем, прошло ли достаточно времени с последнего запроса
  if (currentTime - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = currentTime;
    
    // Отправляем HTTP-запрос
    sim800.println("AT+HTTPACTION=0");
    delay(1000); // Уменьшаем время ожидания

    // Читаем ответ
    sim800.println("AT+HTTPREAD");
    delay(500);

    String response = "";
    while (sim800.available()) {
      response += sim800.readString();
    }

    // Извлекаем команду из ответа
    int index = response.indexOf("\r\n") + 2;
    if (index > 2) { // Проверяем, что нашли начало команды
      String cmd = response.substring(index, response.indexOf("\r\n", index));
      cmd.trim();
      
      // Обрабатываем команду только если она изменилась
      if (cmd != lastCommand) {
        processCommand(cmd);
        lastCommand = cmd;
      }
    }
  }
}

void setupGPRS() {
  sendAT("AT");
  sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendAT("AT+SAPBR=3,1,\"APN\",\"internet\"");
  sendAT("AT+SAPBR=1,1");
  delay(2000);
  sendAT("AT+SAPBR=2,1");
  sendAT("AT+HTTPINIT");
  sendAT("AT+HTTPPARA=\"CID\",1");
  sendAT("AT+HTTPPARA=\"URL\",\"http://13.60.251.21:8000/get-command\"");
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