import cv2
import numpy as np
import urllib.request
import time
import math

stream_url = "http://172.16.12.215/stream"  # anpassen aber statisch

eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')

allowed_cells = [(r, c) for r in range(2, 6) for c in range(4, 11)]
grid_spacing = 50

try:
    stream = urllib.request.urlopen(stream_url)
except Exception as e:
    print(f"Fehler beim Öffnen des Streams: {e}")
    stream = None

bytes_data = b''
previous_area = None
previous_pos = None
previous_time = None

while True:
    if stream is not None:
        try:
            bytes_data += stream.read(1024)
        except Exception as e:
            print(f"readerror: {e}")
            stream = None

    a = bytes_data.find(b'\xff\xd8')
    b = bytes_data.find(b'\xff\xd9')

    if a != -1 and b != -1:
        jpg = bytes_data[a:b+2]
        bytes_data = bytes_data[b+2:]
        img = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
    else:
        img = None

    if img is None:
        print("x Bild")
        continue

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    h, w = gray.shape
    upper_half = gray[0:h//2, :]
    eyes = eye_cascade.detectMultiScale(upper_half, 1.1, 5)
    print(f"Anzahl erkannter Augen: {len(eyes)}")

    # Gitter zeichnen und beschriften
    for y in range(0, h, grid_spacing):
        for x in range(0, w, grid_spacing):
            row, col = y // grid_spacing, x // grid_spacing
            color = (200, 200, 200)
            if (row, col) in allowed_cells:
                color = (0, 255, 255)
            cv2.rectangle(img, (x, y), (x+grid_spacing, y+grid_spacing), color, 1)
            cv2.putText(img, f"{row},{col}", (x+5, y+15),
                        cv2.FONT_HERSHEY_PLAIN, 1, (100, 255, 100), 1)

    if len(eyes) == 0:
        print("Keine Pupillen erkannt")
    else:
        for (x, y, ew, eh) in eyes:
            roi_gray = upper_half[y:y+eh, x:x+ew]
            eye_center = (x + ew // 2, y + eh // 2)
            cell_row = eye_center[1] // grid_spacing
            cell_col = eye_center[0] // grid_spacing

            if (cell_row, cell_col) not in allowed_cells:
                continue

            _, thresh = cv2.threshold(roi_gray, 50, 255, cv2.THRESH_BINARY_INV)
            contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            if contours:
                largest_contour = max(contours, key=cv2.contourArea)
                area = cv2.contourArea(largest_contour)

                if area > 10:
                    pupil_pos = (x + int(np.mean(largest_contour[:, 0, 0])),
                                 y + int(np.mean(largest_contour[:, 0, 1])))

                    current_time = time.time()
                    if previous_pos is not None and previous_time is not None:
                        dx = pupil_pos[0] - previous_pos[0]
                        dy = pupil_pos[1] - previous_pos[1]
                        dt = current_time - previous_time
                        speed = math.sqrt(dx**2 + dy**2) / dt  # px/s
                        cv2.putText(img, f"Speed: {speed:.1f}px/s", (x, y+eh+30),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)
                        print(f"Geschwindigkeit: {speed:.1f} px/s")

                    previous_pos = pupil_pos
                    previous_time = current_time

                    cv2.circle(img, pupil_pos, 5, (0, 255, 0), 2)
                    cv2.rectangle(img, (x, y), (x+ew, y+eh), (255, 0, 0), 2)
                    cv2.putText(img, f"Area: {int(area)}", (x, y-10),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 1)

                    if previous_area is not None:
                        delta = area - previous_area
                        cv2.putText(img, f"Delta: {delta:+.1f}", (x, y+eh+15),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,0), 1)
                        print(f"Pupillengroesse: {int(area)} px² | Änderung: {delta:+.1f} px²")
                    else:
                        print(f"Pupillengroesse: {int(area)} px²")

                    previous_area = area

    cv2.imshow('ESP32-CAM Pupillenerkennung with grid', img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
