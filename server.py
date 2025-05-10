from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import serial
import json
import time

app = FastAPI()

try:
    arduino = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    time.sleep(2)
except serial.SerialException as e:
    print(f"Ошибка подключения к Serial порту: {e}")
    arduino = None

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            command_data = json.loads(data)
            command = command_data["command"]
            if arduino:
                arduino.write(f"{command}\n".encode())
                response = f"Sent: {command}"
            else:
                response = "Serial порт не подключен"
            await websocket.send_text(json.dumps({"status": "ok", "message": response}))
    except WebSocketDisconnect:
        print("Клиент отключился")
    except Exception as e:
        print(f"Ошибка WebSocket: {e}")
        await websocket.send_text(json.dumps({"status": "error", "message": str(e)}))
    finally:
        await websocket.close()