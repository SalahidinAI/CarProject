from fastapi import FastAPI
from fastapi.responses import PlainTextResponse
from pydantic import BaseModel
import logging

# Настройка логирования
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI()

# Хранение текущей команды
current_command = "STOP"

# Модель запроса
class CommandRequest(BaseModel):
    command: str

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
    return {"status": "ok", "command": current_command}
