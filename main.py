import cv2
import mediapipe as mp
import requests
import json

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

cap = cv2.VideoCapture(0)

while True:
    success, img = cap.read()
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
            requests.post(URL, json={"command": gesture})

    cv2.imshow("Image", img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
