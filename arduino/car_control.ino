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
bool gprsConnected = false;

void setup() {
  Serial.begin(9600);
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH); // Включаем LED при старте
  
  sim800.begin(9600);
  steeringServo.attach(servoPin);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  Serial.println(F("Начинаем тест настройки GPRS..."));
  digitalWrite(statusLed, HIGH);
  delay(5000);
  
  // Проверка модуля
  Serial.println(F("\n1. Проверка модуля"));
  String response = sendAT("AT");
  if (response.indexOf("OK") == -1) {
    Serial.println(F("Ошибка: Модуль не отвечает"));
    return;
  }
  
  // Проверка SIM карты
  Serial.println(F("\n2. Проверка SIM карты"));
  response = sendAT("AT+CPIN?");
  if (response.indexOf("+CPIN: READY") == -1) {
    Serial.println(F("Ошибка: SIM карта не готова"));
    Serial.println(F("Проверьте:"));
    Serial.println(F("1. Вставлена ли SIM карта"));
    Serial.println(F("2. Правильно ли вставлена"));
    Serial.println(F("3. Работает ли SIM карта в телефоне"));
    return;
  }
  
  // Проверка регистрации в сети
  Serial.println(F("\n3. Проверка регистрации в сети"));
  int registrationAttempts = 0;
  bool registered = false;
  
  while (!registered && registrationAttempts < 30) { // Пробуем 30 раз (примерно 1 минута)
    // Проверяем силу сигнала
    response = sendAT("AT+CSQ");
    if (response.indexOf("+CSQ:") != -1) {
      int signalStrength = response.substring(response.indexOf("+CSQ:") + 6, response.indexOf(",")).toInt();
      Serial.print(F("Сила сигнала: "));
      Serial.println(signalStrength);
      
      if (signalStrength < 10) {
        Serial.println(F("Внимание: Слабый сигнал!"));
        Serial.println(F("Переместите модуль в место с лучшим сигналом"));
      }
    }
    
    // Проверяем регистрацию
    response = sendAT("AT+CREG?");
    if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
      registered = true;
      Serial.println(F("Зарегистрирован в сети!"));
    } else if (response.indexOf("+CREG: 0,2") != -1) {
      registrationAttempts++;
      Serial.print(F("Поиск сети... Попытка "));
      Serial.println(registrationAttempts);
      
      // Моргаем LED для индикации поиска сети
      digitalWrite(statusLed, HIGH);
      delay(500);
      digitalWrite(statusLed, LOW);
      delay(500);
    } else if (response.indexOf("+CREG: 0,3") != -1) {
      Serial.println(F("Ошибка: Регистрация отклонена"));
      Serial.println(F("Проверьте:"));
      Serial.println(F("1. Не заблокирована ли SIM карта"));
      Serial.println(F("2. Есть ли деньги на счету"));
      return;
    } else {
      Serial.println(F("Ошибка: Неизвестный статус регистрации"));
      Serial.println(F("Попробуйте перезагрузить модуль"));
      return;
    }
    
    delay(2000); // Ждем 2 секунды между проверками
  }
  
  if (!registered) {
    Serial.println(F("Не удалось зарегистрироваться в сети!"));
    Serial.println(F("Проверьте:"));
    Serial.println(F("1. Есть ли покрытие сети"));
    Serial.println(F("2. Работает ли SIM карта в телефоне"));
    Serial.println(F("3. Не заблокирована ли SIM карта"));
    return;
  }
  
  // Проверка оператора
  Serial.println(F("\n4. Проверка оператора"));
  response = sendAT("AT+COPS?");
  Serial.print(F("Оператор: "));
  Serial.println(response);
  
  digitalWrite(statusLed, LOW);
}

void loop() {
  static unsigned long lastCheck = 0;
  static bool gprsInitialized = false;
  static bool httpInitialized = false;
  
  if (millis() - lastCheck >= 5000) {
    lastCheck = millis();
    
    if (!gprsInitialized) {
      Serial.println(F("\nИнициализация GPRS..."));
      
      // Проверяем регистрацию в сети
      Serial.println(F("1. Проверка регистрации в сети"));
      String response = sendAT("AT+CREG?");
      
      if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
        Serial.println(F("Зарегистрирован в сети"));
        
        // Проверяем силу сигнала
        Serial.println(F("2. Проверка силы сигнала"));
        response = sendAT("AT+CSQ");
        
        if (response.indexOf("+CSQ:") != -1) {
          int signalStrength = response.substring(response.indexOf("+CSQ:") + 6, response.indexOf(",")).toInt();
          Serial.print(F("Сила сигнала: "));
          Serial.println(signalStrength);
          
          if (signalStrength >= 10) {
            // Сбрасываем модуль
            Serial.println(F("3. Сброс модуля"));
            sendAT("AT+CFUN=1,1");
            delay(5000);
            
            // Проверяем модуль после сброса
            Serial.println(F("4. Проверка модуля после сброса"));
            response = sendAT("AT");
            if (response.indexOf("OK") == -1) {
              Serial.println(F("Ошибка: Модуль не отвечает после сброса"));
              return;
            }
            
            // Включаем GPRS
            Serial.println(F("5. Включение GPRS"));
            sendAT("AT+CGATT=1");
            delay(3000);
            
            // Проверяем статус GPRS
            Serial.println(F("6. Проверка статуса GPRS"));
            response = sendAT("AT+CGATT?");
            
            if (response.indexOf("+CGATT: 1") != -1) {
              Serial.println(F("GPRS успешно включен!"));
              
              // Настраиваем GPRS
              Serial.println(F("7. Настройка GPRS"));
              sendAT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
              delay(1000);
              
              // Устанавливаем APN
              Serial.println(F("8. Установка APN"));
              sendAT("AT+SAPBR=3,1,\"APN\",\"gprs.megacom.kg\"");
              delay(1000);
              
              // Открываем GPRS соединение
              Serial.println(F("9. Открытие GPRS соединения"));
              sendAT("AT+SAPBR=1,1");
              delay(3000);
              
              // Проверяем статус соединения
              Serial.println(F("10. Проверка статуса соединения"));
              response = sendAT("AT+SAPBR=2,1");
              
              if (response.indexOf("+SAPBR: 1,1") != -1) {
                Serial.println(F("GPRS соединение установлено!"));
                gprsInitialized = true;
                digitalWrite(statusLed, HIGH);
                delay(100);
                digitalWrite(statusLed, LOW);
              } else {
                Serial.println(F("Ошибка установки GPRS соединения!"));
                Serial.println(F("Проверьте:"));
                Serial.println(F("1. Баланс на SIM карте"));
                Serial.println(F("2. Активацию услуги GPRS"));
                Serial.println(F("3. Настройки APN"));
              }
            } else {
              Serial.println(F("GPRS не включен!"));
              Serial.println(F("Попробуйте:"));
              Serial.println(F("1. Проверить SIM карту в телефоне"));
              Serial.println(F("2. Проверить баланс"));
              Serial.println(F("3. Проверить активацию GPRS"));
            }
          } else {
            Serial.println(F("Слабый сигнал!"));
            Serial.println(F("Переместите модуль в место с лучшим сигналом"));
          }
        }
      } else {
        Serial.println(F("Нет регистрации в сети!"));
        Serial.println(F("Ожидание регистрации..."));
      }
    } else if (!httpInitialized) {
      // Инициализация HTTP
      Serial.println(F("\nИнициализация HTTP..."));
      sendAT("AT+HTTPINIT");
      delay(1000);
      
      // Установка параметров HTTP
      Serial.println(F("Настройка HTTP параметров..."));
      sendAT("AT+HTTPPARA=\"CID\",1");
      delay(1000);
      sendAT("AT+HTTPPARA=\"URL\",\"http://13.60.251.21:8000/get-command\"");
      delay(1000);
      
      httpInitialized = true;
      Serial.println(F("HTTP инициализирован!"));
    } else {
      // Проверяем статус GPRS
      Serial.println(F("\nПроверка статуса GPRS..."));
      String response = sendAT("AT+SAPBR=2,1");
      
      if (response.indexOf("+SAPBR: 1,1") != -1) {
        // Отправляем HTTP запрос
        Serial.println(F("Отправка HTTP запроса..."));
        sendAT("AT+HTTPACTION=0");
        delay(2000);
        
        // Читаем ответ
        Serial.println(F("Чтение ответа..."));
        response = sendAT("AT+HTTPREAD");
        
        // Ищем команду в ответе
        if (response.indexOf("FORWARD") != -1) {
          Serial.println(F("Получена команда: FORWARD"));
          digitalWrite(statusLed, HIGH);
          delay(100);
          digitalWrite(statusLed, LOW);
        } else if (response.indexOf("BACKWARD") != -1) {
          Serial.println(F("Получена команда: BACKWARD"));
          digitalWrite(statusLed, HIGH);
          delay(100);
          digitalWrite(statusLed, LOW);
        } else if (response.indexOf("LEFT") != -1) {
          Serial.println(F("Получена команда: LEFT"));
          digitalWrite(statusLed, HIGH);
          delay(100);
          digitalWrite(statusLed, LOW);
        } else if (response.indexOf("RIGHT") != -1) {
          Serial.println(F("Получена команда: RIGHT"));
          digitalWrite(statusLed, HIGH);
          delay(100);
          digitalWrite(statusLed, LOW);
        } else if (response.indexOf("STOP") != -1) {
          Serial.println(F("Получена команда: STOP"));
          digitalWrite(statusLed, HIGH);
          delay(100);
          digitalWrite(statusLed, LOW);
        } else {
          Serial.println(F("Нет новых команд"));
        }
      } else {
        Serial.println(F("GPRS соединение потеряно!"));
        gprsInitialized = false;
        httpInitialized = false;
      }
    }
  }
}

String sendAT(String cmd) {
  Serial.print(F("Отправка команды: "));
  Serial.println(cmd);
  
  sim800.println(cmd);
  delay(2000);
  
  String response = "";
  while (sim800.available()) {
    String line = sim800.readString();
    response += line;
    Serial.print(F("Ответ: "));
    Serial.println(line);
  }
  Serial.println();
  return response;
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