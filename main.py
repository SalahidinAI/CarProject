import cv2
import mediapipe as mp
import asyncio
import websockets
import json
import platform

# Инициализация
mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
hands = mp_hands.Hands(max_num_hands=2, min_detection_confidence=0.7, min_tracking_confidence=0.7)

tip_ids = [4, 8, 12, 16, 20]  # [большой, указательный, средний, безымянный, мизинец]

def get_finger_status(lm_list, hand_type):
    fingers = []
    if hand_type == "Right":
        fingers.append(1 if lm_list[tip_ids[0]][1] < lm_list[tip_ids[0] - 1][1] else 0)
    else:
        fingers.append(1 if lm_list[tip_ids[0]][1] > lm_list[tip_ids[0] - 1][1] else 0)
    for i in range(1, 5):
        fingers.append(1 if lm_list[tip_ids[i]][2] < lm_list[tip_ids[i] - 2][2] else 0)
    return fingers

def recognize_command(fingers, hand_type):
    if fingers == [0, 1, 0, 0, 0]:
        return "FORWARD"
    elif hand_type == "Right" and fingers == [0, 1, 1, 0, 0]:
        return "RIGHT"
    elif hand_type == "Right" and fingers == [1, 1, 0, 0, 0]:
        return "LEFT"
    elif hand_type == "Left" and fingers == [1, 1, 0, 0, 0]:
        return "RIGHT"
    elif hand_type == "Left" and fingers == [0, 1, 1, 0, 0]:
        return "LEFT"
    elif fingers == [1, 1, 1, 1, 1]:
        return "BACKWARD"
    else:
        return "STOP"

async def send_command(command):
    uri = "ws://13.60.251.21:8000/ws"  # Ваш IP EC2
    try:
        async with websockets.connect(uri) as websocket:
            await websocket.send(json.dumps({"command": command}))
    except Exception as e:
        print(f"Ошибка соединения с WebSocket: {e}")

cap = cv2.VideoCapture(0)
prev_command = "STOP"

async def main():
    global prev_command
    try:
        while True:
            success, img = cap.read()
            if not success:
                print("Не удалось захватить кадр с камеры")
                break
            img = cv2.flip(img, 1)
            img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            results = hands.process(img_rgb)

            final_command = "STOP"
            hand_data = []

            if results.multi_hand_landmarks and results.multi_handedness:
                for handLms, hand_handedness in zip(results.multi_hand_landmarks, results.multi_handedness):
                    lm_list = []
                    h, w, _ = img.shape
                    for id, lm in enumerate(handLms.landmark):
                        cx, cy = int(lm.x * w), int(lm.y * h)
                        lm_list.append((id, cx, cy))

                    hand_type = hand_handedness.classification[0].label
                    fingers = get_finger_status(lm_list, hand_type)
                    command = recognize_command(fingers, hand_type)

                    hand_data.append({
                        "type": hand_type,
                        "command": command,
                        "landmarks": handLms
                    })

            if len(hand_data) == 1:
                final_command = hand_data[0]["command"]
                mp_draw.draw_landmarks(img, hand_data[0]["landmarks"], mp_hands.HAND_CONNECTIONS)
            elif len(hand_data) == 2:
                for hand in hand_data:
                    if hand["type"] == "Right":
                        final_command = hand["command"]
                        mp_draw.draw_landmarks(img, hand["landmarks"], mp_hands.HAND_CONNECTIONS)
                        break
                else:
                    final_command = hand_data[0]["command"]
                    mp_draw.draw_landmarks(img, hand_data[0]["landmarks"], mp_hands.HAND_CONNECTIONS)

            if final_command != prev_command:
                await send_command(final_command)
                prev_command = final_command

            cv2.putText(img, f'Command: {final_command}', (10, 60),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0), 3)
            cv2.imshow("Hand Control", img)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    except KeyboardInterrupt:
        print("Программа завершена пользователем")
    finally:
        cap.release()
        cv2.destroyAllWindows()

if platform.system() == "Emscripten":
    asyncio.ensure_future(main())
else:
    if __name__ == "__main__":
        asyncio.run(main())