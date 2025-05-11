import requests

r = requests.get("http://13.60.251.21:8000/get-command")

print(f"GET command: {r.text}")
