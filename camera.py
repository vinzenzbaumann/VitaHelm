import cv2
import numpy as np
import urllib.request

stream_url = "http://172.16.12.215/stream"

# Öffne den Stream über urllib und lese ihn als Bytes ein
stream = urllib.request.urlopen(stream_url)
bytes_data = b''

while True:
    bytes_data += stream.read(1024)
    a = bytes_data.find(b'\xff\xd8')  # JPEG start
    b = bytes_data.find(b'\xff\xd9')  # JPEG end

    if a != -1 and b != -1:
        jpg = bytes_data[a:b+2]
        bytes_data = bytes_data[b+2:]
        img = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)

        if img is None:
            continue

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Pupillenerkennung via Haarcascade (Augen)
        eyes_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')
        eyes = eyes_cascade.detectMultiScale(gray, 1.1, 5)

        if len(eyes) == 0:
            print("Keine Pupillen erkannt")
        else:
            for (x, y, w, h) in eyes:
                roi_gray = gray[y:y+h, x:x+w]
                minVal, maxVal, minLoc, maxLoc = cv2.minMaxLoc(roi_gray)
                pupil_pos = (x + maxLoc[0], y + maxLoc[1])

                # Kreis um die Pupille zeichnen
                cv2.circle(img, pupil_pos, 5, (0, 255, 0), 2)
                cv2.rectangle(img, (x, y), (x+w, y+h), (255, 0, 0), 2)

                print(f"Pupille erkannt bei Position: {pupil_pos}")

        cv2.imshow('ESP32-CAM Pupillenerkennung', img)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

cv2.destroyAllWindows()

