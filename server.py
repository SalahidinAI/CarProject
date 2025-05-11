from fastapi import FastAPI
from fastapi.responses import PlainTextResponse
from pydantic import BaseModel
import logging
import serial
import time

# Настройка логирования
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI()

# Хранение текущей команды
current_command = "STOP"

# Модель запроса
class CommandRequest(BaseModel):
    command: str

# Инициализация Serial порта для Arduino
try:
    arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    time.sleep(2)  # Даем Arduino время на перезагрузку
    logger.info("Arduino connected successfully")
except Exception as e:
    logger.error(f"Failed to connect to Arduino: {e}")
    arduino = None

def send_to_arduino(command: str):
    if arduino and arduino.is_open:
        try:
            arduino.write(f"{command}\n".encode())
            logger.info(f"Command sent to Arduino: {command}")
        except Exception as e:
            logger.error(f"Error sending command to Arduino: {e}")

# Получить текущую команду
@app.get("/get-command", response_class=PlainTextResponse)
async def get_command():
    logger.info(f"GET request received. Current command: {current_command}")
    return current_command

# Установить новую команду
@app.post("/set-command")
async def set_command(data: CommandRequest):
    global current_command
    logger.info(f"POST request received with command: {data.command}")
    current_command = data.command
    logger.info(f"Command updated to: {current_command}")
    
    # Отправляем команду на Arduino
    send_to_arduino(current_command)
    
    return {"status": "ok", "command": current_command}

# Очистка при завершении работы
@app.on_event("shutdown")
async def shutdown_event():
    if arduino and arduino.is_open:
        arduino.close()
        logger.info("Arduino connection closed")
