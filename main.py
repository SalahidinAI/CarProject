import cv2
import mediapipe as mp
import requests
import json
import logging
import time

# Настройка логирования
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
mp_draw = mp.solutions.drawing_utils

tip_ids = [4, 8, 12, 16, 20]

URL = "http://13.60.251.21:8000/set-command"  # Замените на ваш IP

def get_hand_gesture(fingers):
    if fingers == [0, 1, 0, 0, 0]:
        return "FORWARD"
    elif fingers == [0, 1, 1, 0, 0]:
        return "BACKWARD"
    elif fingers == [0, 0, 0, 0, 1]:
        return "RIGHT"
    elif fingers == [1, 0, 0, 0, 0]:
        return "LEFT"
    else:
        return "STOP"

def send_command(command):
    try:
        response = requests.post(URL, json={"command": command}, timeout=1)
        response.raise_for_status()
        logger.info(f"Command sent successfully: {command}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Error sending command: {e}")
        return False

cap = cv2.VideoCapture(0)
last_command = None
last_command_time = 0
COMMAND_COOLDOWN = 0.5  # Задержка между командами в секундах

while True:
    success, img = cap.read()
    if not success:
        logger.error("Failed to capture image")
        continue

    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    result = hands.process(img_rgb)

    if result.multi_hand_landmarks:
        for hand_landmark in result.multi_hand_landmarks:
            lm_list = []
            for id, lm in enumerate(hand_landmark.landmark):
                h, w, _ = img.shape
                lm_list.append((int(lm.x * w), int(lm.y * h)))
            fingers = []
            if lm_list[tip_ids[0]][0] > lm_list[tip_ids[0] - 1][0]:
                fingers.append(1)
            else:
                fingers.append(0)
            for id in range(1, 5):
                fingers.append(1 if lm_list[tip_ids[id]][1] < lm_list[tip_ids[id] - 2][1] else 0)

            gesture = get_hand_gesture(fingers)
            current_time = time.time()
            
            # Отправляем команду только если она изменилась и прошло достаточно времени
            if gesture != last_command and (current_time - last_command_time) >= COMMAND_COOLDOWN:
                if send_command(gesture):
                    last_command = gesture
                    last_command_time = current_time

    cv2.imshow("Image", img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
