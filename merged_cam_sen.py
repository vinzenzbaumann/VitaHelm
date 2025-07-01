import socket
import cv2
import numpy as np
import urllib.request
import time
import math
import os

# --- Einstellungen ---
UDP_IP = "0.0.0.0"
UDP_PORT = 42190
STREAM_URL = "http://172.16.12.215/stream"

eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')
ALLOWED_CELLS = [(r, c) for r in range(2, 6) for c in range(4, 11)]
GRID_SPACING = 50

# --- Hilfsfunktionen ---

def setup_udp_socket(ip=UDP_IP, port=UDP_PORT):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((ip, port))
    sock.settimeout(0.01)
    print(f"UDP-Empfänger gestartet auf {ip}:{port}\n")
    return sock

def open_stream(url=STREAM_URL):
    try:
        stream = urllib.request.urlopen(url)
        print(f"Stream geöffnet: {url}")
        return stream
    except Exception as e:
        print(f"Fehler beim Öffnen des Streams: {e}")
        return None

def parse_udp_data(data_str):
    result = {}
    parts = data_str.strip().split(",")
    for part in parts:
        if ':' not in part:
            continue
        key, value = part.split(":", 1)
        try:
            if key in ["X", "Y", "Z", "MicTrigger", "MicAnalog", "BPM", "AvgBPM"]:
                result[key] = int(value)
            elif key == "BreathRate":
                result[key] = float(value)
        except:
            continue
    return result

def calculate_accel_magnitude(x, y, z):
    return math.sqrt(x*x + y*y + z*z)

def clamp(value, min_val, max_val):
    return max(min_val, min(max_val, value))

def map_value(value, in_min, in_max, out_min, out_max):
    # Linear mapping mit Clamping
    value = clamp(value, in_min, in_max)
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min)

def calculate_erregungswert(mic_trigger, speed, area, delta, mic_analog=0,
                            breath_rate=0.0, accel_magnitude=0.0, avg_bpm=0):

    # Wertebereiche clampen bzw. mappen
    mic_analog = clamp(mic_analog, 0, 4100)
    breath_rate = clamp(breath_rate, 5, 50)
    accel_magnitude = clamp(accel_magnitude, 0, 8000)
    avg_bpm = clamp(avg_bpm, 40, 200)

    # Scores berechnen (Skalierung bleibt ähnlich wie vorher)
    mic_score = 200 if mic_trigger else 0
    mic_analog_score = map_value(mic_analog, 0, 4100, 0, 150)
    speed_score = min(speed * 5, 150)
    area_score = min(max((area - 100) * 1.2, 0), 150)
    delta_score = min(max(delta, -200), 200) + 200
    accel_score = map_value(accel_magnitude, 0, 8000, 0, 200)

    # BreathRate Scores
    if 12 <= breath_rate <= 20:
        breath_rate_score = 150
    elif 8 <= breath_rate < 12 or 20 < breath_rate <= 24:
        breath_rate_score = 100
    else:
        breath_rate_score = 50

    # Avg BPM Scores
    if 60 <= avg_bpm <= 100:
        bpm_score = 150
    elif 50 <= avg_bpm < 60 or 100 < avg_bpm <= 110:
        bpm_score = 100
    else:
        bpm_score = 50

    gesamtwert = int(
        0.10 * mic_score +
        0.10 * mic_analog_score +
        0.10 * speed_score +
        0.10 * area_score +
        0.10 * delta_score +
        0.20 * breath_rate_score +
        0.10 * accel_score +
        0.20 * bpm_score
    )
    return max(0, min(gesamtwert, 599))


def draw_grid(img, spacing=GRID_SPACING, allowed=ALLOWED_CELLS):
    h, w = img.shape[:2]
    for y in range(0, h, spacing):
        for x in range(0, w, spacing):
            row, col = y // spacing, x // spacing
            color = (0, 255, 255) if (row, col) in allowed else (200, 200, 200)
            cv2.rectangle(img, (x, y), (x + spacing, y + spacing), color, 1)
            cv2.putText(img, f"{row},{col}", (x + 5, y + 15),
                        cv2.FONT_HERSHEY_PLAIN, 1, (100, 255, 100), 1)


# --- Hauptprogramm ---

def main():
    sock = setup_udp_socket()
    stream = open_stream()

    csv_file = "daten_log.csv"
    # Header schreiben, falls Datei nicht existiert oder leer
    if not os.path.exists(csv_file) or os.path.getsize(csv_file) == 0:
        with open(csv_file, "w") as f:
            f.write("Timestamp,Erregungswert,MicTrigger,MicAnalog,BreathRate,BPM,Speed,Area,Delta,AccelMagnitude\n")

    bytes_data = b''
    previous_area = None
    previous_pos = None
    previous_time = None

    mic_trigger = 0
    mic_analog = 0
    breath_rate = 0.0
    bpm = 0
    accel_magnitude = 0.0
    erregungswert = 0

    addr = None

    last_udp_process_time = 0

    try:
        while True:
            current_time = time.time()

            # UDP Empfang: Alle Pakete lesen, aber nur 1x pro Sekunde auswerten
            try:
                while True:
                    data, addr = sock.recvfrom(1024)
                    decoded = data.decode("utf-8", errors="ignore").strip()

                    if current_time - last_udp_process_time >= 1.0:
                        udp_values = parse_udp_data(decoded)
                        if udp_values:
                            mic_trigger = udp_values.get("MicTrigger", mic_trigger)
                            mic_analog = udp_values.get("MicAnalog", mic_analog)
                            breath_rate = udp_values.get("BreathRate", breath_rate)
                            bpm = udp_values.get("BPM", bpm)
                            x = udp_values.get("X", 0)
                            y = udp_values.get("Y", 0)
                            z = udp_values.get("Z", 0)
                            accel_magnitude = calculate_accel_magnitude(x, y, z)
                        last_udp_process_time = current_time

            except socket.timeout:
                pass
            except Exception as e:
                print(f"Fehler beim UDP-Empfang: {e}")

            # MJPEG Stream lesen
            if stream:
                try:
                    bytes_data += stream.read(8192)
                except Exception as e:
                    print(f"Fehler beim Lesen des Streams: {e}")
                    stream = None

            a = bytes_data.find(b'\xff\xd8')
            b = bytes_data.find(b'\xff\xd9')

            if a != -1 and b != -1 and b > a:
                jpg = bytes_data[a:b+2]
                bytes_data = bytes_data[b+2:]
                img = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
                img = cv2.flip(img, 0)
            else:
                img = None

            if img is None:
                continue

            # Pupillenerkennung
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            h, w = gray.shape
            upper_half = gray[0:h//2, :]

            eyes = eye_cascade.detectMultiScale(upper_half, scaleFactor=1.1, minNeighbors=5)
            draw_grid(img)
            speed = 0
            area = 0
            delta = 0

            if len(eyes) == 0:
                print("Keine Pupillen erkannt")
            else:
                for (ex, ey, ew, eh) in eyes:
                    roi_gray = upper_half[ey:ey+eh, ex:ex+ew]
                    eye_center = (ex + ew // 2, ey + eh // 2)
                    cell_row = eye_center[1] // GRID_SPACING
                    cell_col = eye_center[0] // GRID_SPACING

                    if (cell_row, cell_col) not in ALLOWED_CELLS:
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
                                    speed = math.sqrt(dx*dx + dy*dy) / dt

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

                            erregungswert = calculate_erregungswert(
                                mic_trigger, speed, area, delta,
                                mic_analog, breath_rate, accel_magnitude, bpm)

            # CSV-Zeile schreiben
            timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
            csv_line = f"{timestamp},{erregungswert},{mic_trigger},{mic_analog},{breath_rate},{bpm},{speed:.1f},{int(area)},{delta:.1f},{accel_magnitude:.1f}\n"
            with open(csv_file, "a") as f:
                f.write(csv_line)

            print(f"\rErregungswert: {erregungswert:<3} | MIC={mic_trigger} MicAnalog={mic_analog} " 
                  f"BreathRate={breath_rate:.1f} BPM={bpm} Speed={speed:.1f} Area={int(area)} "
                  f"Delta={delta:+.1f} AccelMag={accel_magnitude:.1f}", end="")
            
            # Antwort an Sender senden
            if addr is not None:
                antwort = f"Erregung:{erregungswert}"
                try:
                    sock.sendto(antwort.encode('utf-8'), addr)
                except Exception as e:
                    print(f"\nFehler beim Senden: {e}")

            # Anzeige
            cv2.imshow('ESP32-CAM Pupillenerkennung with grid', img)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except KeyboardInterrupt:
        print("\nManuell beendet.")
    finally:
        sock.close()
        cv2.destroyAllWindows()
        print("Socket geschlossen und Fenster geschlossen.")


if __name__ == "__main__":
    main()
