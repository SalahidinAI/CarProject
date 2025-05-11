#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial sim800(10, 11); // RX, TX
Servo steeringServo;

const int motorPin1 = 4;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 7;
const int servoPin = 9;
const int statusLed = 13; // Встроенный LED на Arduino

String lastCommand = "";
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 2000; // Интервал запросов 2 секунды

void setup() {
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH); // Включаем LED при старте
  
  sim800.begin(9600);
  steeringServo.attach(servoPin);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  Serial.println("Starting...");
  delay(3000); // Ждем подключения SIM
  setupGPRS();
  digitalWrite(statusLed, LOW); // Выключаем LED после инициализации
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = currentTime;
    digitalWrite(statusLed, HIGH); // Мигаем LED при запросе
    
    Serial.println("Sending HTTP request...");
    sim800.println("AT+HTTPACTION=0");
    delay(1000);

    Serial.println("Reading response...");
    sim800.println("AT+HTTPREAD");
    delay(500);

    String response = "";
    while (sim800.available()) {
      char c = sim800.read();
      response += c;
      Serial.write(c); // Выводим ответ в Serial Monitor
    }

    int index = response.indexOf("\r\n") + 2;
    if (index > 2) {
      String cmd = response.substring(index, response.indexOf("\r\n", index));
      cmd.trim();
      Serial.print("Received command: ");
      Serial.println(cmd);
      
      if (cmd != lastCommand) {
        Serial.print("Executing new command: ");
        Serial.println(cmd);
        processCommand(cmd);
        lastCommand = cmd;
        // Мигаем LED дважды при получении новой команды
        digitalWrite(statusLed, LOW);
        delay(100);
        digitalWrite(statusLed, HIGH);
        delay(100);
        digitalWrite(statusLed, LOW);
      }
    } else {
      Serial.println("No valid command received");
    }
    digitalWrite(statusLed, LOW);
  }
}

void setupGPRS() {
  Serial.println("Setting up GPRS...");
  
  sendAT("AT");
  sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  sendAT("AT+SAPBR=3,1,\"APN\",\"internet\"");
  sendAT("AT+SAPBR=1,1");
  delay(2000);
  sendAT("AT+SAPBR=2,1");
  sendAT("AT+HTTPINIT");
  sendAT("AT+HTTPPARA=\"CID\",1");
  sendAT("AT+HTTPPARA=\"URL\",\"http://13.60.251.21:8000/get-command\"");
  
  Serial.println("GPRS setup complete");
}

void sendAT(String cmd) {
  Serial.print("Sending AT command: ");
  Serial.println(cmd);
  
  sim800.println(cmd);
  delay(1000);
  
  Serial.print("Response: ");
  while (sim800.available()) {
    String response = sim800.readString();
    Serial.println(response);
  }
}

void processCommand(String cmd) {
  Serial.print("Processing command: ");
  Serial.println(cmd);
  
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