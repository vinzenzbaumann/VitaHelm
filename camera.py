import cv2
import numpy as np
import requests
from datetime import datetime

# IP der Kamera
CAM_URL = "http://192.168.2.134"

# Vorherigen Zeitstempel speichern
last_timestamp = None

# Lade ein Bild von der Kamera
def get_frame():
    try:
        response = requests.get(f"{CAM_URL}/capture", timeout=2)
        img_array = np.asarray(bytearray(response.content), dtype=np.uint8)
        frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
        return frame
    except:
        print("Fehler beim Abrufen des Bildes.")
        return None

# Pupillenbewegung analysieren
def detect_pupil_movement(frame, debug=False):
    global last_timestamp

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    eyes_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')

    eyes = eyes_cascade.detectMultiScale(gray, 1.1, 4)

    for (x, y, w, h) in eyes:
        eye = gray[y:y+h, x:x+w]
        _, thresh = cv2.threshold(eye, 30, 255, cv2.THRESH_BINARY_INV)
        contours, _ = cv2.findContours(thresh, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # Größter dunkler Bereich = Pupille
        if contours:
            pupil = max(contours, key=cv2.contourArea)
            (cx, cy), radius = cv2.minEnclosingCircle(pupil)
            center = (int(x + cx), int(y + cy))

            # Zeitstempel berechnen
            current_timestamp = datetime.now()
            if last_timestamp:
                time_diff = current_timestamp - last_timestamp
                print(f"Zeitdifferenz zwischen Frames: {time_diff}")

            last_timestamp = current_timestamp

            # Zeitstempel und Position ausgeben
            print(f"Zeit: {current_timestamp}, Position: {center}")

            if debug:
                cv2.circle(frame, center, int(radius), (0, 255, 0), 2)
                cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)

    return frame

# Hauptloop
def main():
    print("Starte Pupillenbewegungserkennung mit Zeitstempel. Drücke 'q' zum Beenden.")

    while True:
        frame = get_frame()
        if frame is not None:
            result = detect_pupil_movement(frame, debug=True)
            cv2.imshow("Pupillenanalyse", result)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
