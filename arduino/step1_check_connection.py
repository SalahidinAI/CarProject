import requests

try:
    r = requests.get("http://13.60.251.21:8000/get-command", timeout=5)
    print(f"GET OK: {r.status_code}, Text: {r.text}")
except Exception as e:
    print("Connection failed:", e)
