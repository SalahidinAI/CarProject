from fastapi import FastAPI
from fastapi.responses import PlainTextResponse
from pydantic import BaseModel

app = FastAPI()

# Хранение текущей команды
current_command = "STOP"

# Модель запроса
class CommandRequest(BaseModel):
    command: str

# Получить текущую команду
@app.get("/get-command", response_class=PlainTextResponse)
async def get_command():
    return current_command

# Установить новую команду
@app.post("/set-command")
async def set_command(data: CommandRequest):
    global current_command
    current_command = data.command
    return {"status": "ok", "command": current_command}
