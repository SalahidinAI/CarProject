import requests

data = {"command": "FORWARD"}

r = requests.post("http://13.60.251.21:8000/set-command", json=data)

print(f"POST status: {r.status_code}, response: {r.text}")
