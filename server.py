from fastapi import FastAPI
from fastapi.responses import PlainTextResponse

app = FastAPI()

current_command = "STOP"

@app.get("/get-command", response_class=PlainTextResponse)
async def get_command():
    return current_command

@app.post("/set-command")
async def set_command(data: dict):
    global current_command
    current_command = data.get("command", "STOP")
    return {"status": "ok", "command": current_command}
