import socket
import sys
import cv2
import numpy as np
import urllib.request
import time
import math

UDP_IP = "0.0.0.0"
UDP_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"UDP-Empfänger gestartet auf Port {UDP_PORT}...\n")

stream_url = "http://172.16.12.215/stream"  # anpassen

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

# Initialwerte (damit keine Fehler bei erstmaligem Aufruf)
bpm = 60
avg_bpm = 60
mic_trigger = 0
mic_analog = 0
breath_rate = 0.0
speed = 0
area = 0
delta = 0
addr = None  # Senderadresse für Antwort

def berechne_erregungswert(bpm, avg_bpm, mic_trigger, speed, area, delta, mic_analog=0, breath_rate=0.0):
    bpm_score = min(max((bpm - 50) * 1.5, 0), 100)
    avg_bpm_score = min(max((avg_bpm - 50) * 1.2, 0), 100)
    mic_score = 50 if mic_trigger else 0
    speed_score = min(max(speed * 1.5, 0), 100)
    area_score = min(max((area - 100) / 4, 0), 100)
    delta_score = min(max(delta / 2, -50), 50) + 50

    # Scores für MicAnalog und BreathRate (Beispielwerte, anpassbar)
    mic_analog_score = min(max(mic_analog / 10, 0), 50)  # z.B. 0-50 Punkte
    breath_rate_score = 0
    # Atemfrequenz normal (12-20 BPM) gibt Bonus, sonst weniger
    if 12 <= breath_rate <= 20:
        breath_rate_score = 30
    elif breath_rate > 20:
        breath_rate_score = 10
    else:
        breath_rate_score = 0

    gesamtwert = int(
        0.18 * bpm_score +
        0.18 * avg_bpm_score +
        0.12 * mic_score +
        0.12 * speed_score +
        0.12 * area_score +
        0.12 * delta_score +
        0.08 * mic_analog_score +
        0.08 * breath_rate_score
    )
    return max(0, min(gesamtwert, 599))

try:
    while True:
        # 1. UDP-Daten empfangen
        sock.settimeout(0.01)
        try:
            data, addr = sock.recvfrom(1024)
            decoded = data.decode("utf-8", errors="ignore").strip()

            if "X:" in decoded and "MicTrigger:" in decoded:
                parts = decoded.split(",")
                try:
                    x = int(parts[0].split(":")[1])
                    y = int(parts[1].split(":")[1])
                    z = int(parts[2].split(":")[1])
                    mic_trigger = int(parts[3].split(":")[1])

                    bpm = 60
                    avg_bpm = 60
                    mic_analog = 0
                    breath_rate = 0.0

                    for part in parts:
                        if "MicAnalog:" in part:
                            mic_analog = int(part.split(":")[1])
                        elif "BreathRate:" in part:
                            breath_rate = float(part.split(":")[1])
                        elif "BPM:" in part and "AvgBPM" not in part:
                            bpm = int(part.split(":")[1])
                        elif "AvgBPM:" in part:
                            avg_bpm = int(part.split(":")[1])

                except Exception as e:
                    print(f"⚠️ Fehler beim Parsen UDP-Daten: {e}")

        except socket.timeout:
            pass

        # 2. Video-Stream verarbeiten (Pupillenerkennung)
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

        # Gitter zeichnen
        for y in range(0, h, grid_spacing):
            for x in range(0, w, grid_spacing):
                row, col = y // grid_spacing, x // grid_spacing
                color = (200, 200, 200)
                if (row, col) in allowed_cells:
                    color = (0, 255, 255)
                cv2.rectangle(img, (x, y), (x+grid_spacing, y+grid_spacing), color, 1)
                cv2.putText(img, f"{row},{col}", (x+5, y+15),
                            cv2.FONT_HERSHEY_PLAIN, 1, (100, 255, 100), 1)

        speed = 0  # Default zurücksetzen, falls keine Pupillenbewegung erkannt wird

        if len(eyes) == 0:
            print("Keine Pupillen erkannt")
        else:
            for (ex, ey, ew, eh) in eyes:
                roi_gray = upper_half[ey:ey+eh, ex:ex+ew]
                eye_center = (ex + ew // 2, ey + eh // 2)
                cell_row = eye_center[1] // grid_spacing
                cell_col = eye_center[0] // grid_spacing

                if (cell_row, cell_col) not in allowed_cells:
                    continue

                _, thresh = cv2.threshold(roi_gray, 50, 255, cv2.THRESH_BINARY_INV)
                contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

                if contours:
                    largest_contour = max(contours, key=cv2.contourArea)
                    area_new = cv2.contourArea(largest_contour)

                    if area_new > 10:
                        pupil_pos = (ex + int(np.mean(largest_contour[:, 0, 0])),
                                     ey + int(np.mean(largest_contour[:, 0, 1])))

                        current_time = time.time()
                        if previous_pos is not None and previous_time is not None:
                            dx = pupil_pos[0] - previous_pos[0]
                            dy = pupil_pos[1] - previous_pos[1]
                            dt = current_time - previous_time
                            if dt > 0:
                                speed = math.sqrt(dx**2 + dy**2) / dt  # px/s

                        previous_pos = pupil_pos
                        previous_time = current_time

                        area = area_new
                        delta = area - previous_area if previous_area is not None else 0
                        previous_area = area

                        cv2.circle(img, pupil_pos, 5, (0, 255, 0), 2)
                        cv2.rectangle(img, (ex, ey), (ex+ew, ey+eh), (255, 0, 0), 2)
                        cv2.putText(img, f"Area: {int(area)}", (ex, ey-10),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 1)
                        cv2.putText(img, f"Speed: {speed:.1f}px/s", (ex, ey+eh+30),
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)

        erregungswert = berechne_erregungswert(bpm, avg_bpm, mic_trigger, speed, area, delta, mic_analog, breath_rate)
        print(f"\rErregungswert: {erregungswert:<3} | BPM={bpm} AvgBPM={avg_bpm} MIC={mic_trigger} MicAnalog={mic_analog} BreathRate={breath_rate:.1f} Speed={speed:.1f} Area={int(area)} Delta={delta:+.1f}", end="")

        # UDP-Antwort an ESP32 (wenn Adresse bekannt)
        if addr is not None:
            antwort = f"Erregung:{erregungswert}"
            try:
                sock.sendto(antwort.encode('utf-8'), addr)
            except Exception as e:
                print(f"\nFehler beim Senden der Antwort: {e}")

        cv2.imshow('ESP32-CAM Pupillenerkennung with grid', img)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\nManuell beendet.")
finally:
    sock.close()
    cv2.destroyAllWindows()
    print("Socket geschlossen.")
