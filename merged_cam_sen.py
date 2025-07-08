import socket
import cv2
import numpy as np
import urllib.request
import time
import math
import os
import csv
from datetime import datetime


# --- Einstellungen ---
UDP_IP = "0.0.0.0"
UDP_PORT = 42190
STREAM_URL = "http://172.16.12.215/stream"

# Konfiguration
GRID_SPACING = 50
# Erlaubte Zellen (reihen, spalten):
# Reihe 2, Spalten 3 bis 9 (10 exklusiv)
# Reihe 3, Spalten 2 bis 9
# Reihe 4, Spalten 2 bis 8
ALLOWED_CELLS = [(2, c) for c in range(3, 10)] + \
                [(3, c) for c in range(2, 10)] + \
                [(4, c) for c in range(2, 9)]

SCALE_FACTOR_ADXL_8G = 0.0156

eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')

# UDP Socket Setup
def setup_udp_socket(ip=UDP_IP, port=UDP_PORT):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((ip, port))
    sock.settimeout(0.01)
    print(f"UDP-Empfänger gestartet auf {ip}:{port}\n")
    return sock
#Entzerrung

import cv2
import numpy as np




# Kamera-Stream 

def open_stream(url=STREAM_URL):
    try:
        stream = urllib.request.urlopen(url, timeout=1)
        print(f"Stream geöffnet: {url}")
        return stream
    except Exception as e:
        print(f"Stream-Fehler: {e}")
        return None

# UDP Daten parsen
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

# Beschleunigungsmagnitude

def calculate_accel_magnitude(x, y, z, xold, yold, zold):
    deltax = (x - xold) / 100
    deltay = (y - yold) / 100
    deltaz = (z - zold) / 100
    return math.sqrt(deltax**2 + deltay**2 + deltaz**2)

# Wert clampen/mappen
def clamp(value, min_val, max_val):
    return max(min_val, min(max_val, value))

def map_value(value, in_min, in_max, out_min, out_max):
    value = clamp(value, in_min, in_max)
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min)

# Erregungswert Berechnung

def calculate_erregungswert(speed, area, delta, breath_rate=0.0, accel_magnitude=0.0, avg_bpm=0):
    breath_rate = clamp(breath_rate, 5, 50)
    avg_bpm = clamp(avg_bpm, 40, 200)

    speed_score = map_value(speed, -100, 100, 0, 30)
    area_score = map_value(area, 0, 4500, 0, 30)
    delta_score = map_value(delta, -2000, 2000, 0, 60)
    accel_score = map_value(accel_magnitude, 0, 5.2, 0, 120)
    breath_rate_score = map_value(breath_rate, 4, 50, 0, 120)
    bpm_score = map_value(avg_bpm, 40, 200, 0, 240)

    gesamtwert = int(speed_score + delta_score + area_score + accel_score + bpm_score + breath_rate_score)

    print(f"AreaScore: {area_score} DeltaScore: {delta_score} AccelScore: {accel_score} "
          f"BPMScore: {bpm_score:.1f} BreathRateScore: {breath_rate_score:.1f}", end="")

    csv_file = "score_log.csv"
    file_exists = os.path.exists(csv_file)
    with open(csv_file, mode='a', newline='') as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow([
                "Timestamp", "Speed", "Area", "Delta", "BreathRate", "AccelMagnitude", "BPM",
                "SpeedScore", "AreaScore", "DeltaScore", "AccelScore", "BreathRateScore", "BPMScore",
                "Erregungswert"])
        writer.writerow([
            datetime.now().isoformat(timespec='seconds'),
            round(speed, 2), round(area, 2), round(delta, 2),
            round(breath_rate, 2), round(accel_magnitude, 2), round(avg_bpm, 2),
            round(speed_score, 2), round(area_score, 2), round(delta_score, 2),
            round(accel_score, 2), round(breath_rate_score, 2), round(bpm_score, 2),
            gesamtwert])

    return max(0, min(gesamtwert, 599))

# Raster zeichnen
def draw_grid(img, spacing=GRID_SPACING, allowed=ALLOWED_CELLS):
    h, w = img.shape[:2]
    for y in range(0, h, spacing):
        for x in range(0, w, spacing):
            row, col = y // spacing, x // spacing
            color = (0, 255, 0) if (row, col) in allowed else (200, 200, 200)
            cv2.rectangle(img, (x, y), (x + spacing, y + spacing), color, 1)
            cv2.putText(img, f"{row},{col}", (x + 5, y + 15),
                        cv2.FONT_HERSHEY_PLAIN, 1, (0, 255, 0), 1)

# Prüfe, ob Rechteck innerhalb erlaubter Zellen liegt
def is_in_allowed_cells(x, y, w, h, spacing=GRID_SPACING):
    # Mittelpkt des Rechtecks
    cx = x + w // 2
    cy = y + h // 2
    row = cy // spacing
    col = cx // spacing
    return (row, col) in ALLOWED_CELLS

# Hauptfunktion
def main():

    sock = setup_udp_socket()
    stream = open_stream()
    # erregungswert Glättung
    smoothed_erregungswert = None
    smoothing_factor = 0.2

    #Glättung


    smoothed_speed = None
    smoothed_area = None
    smoothed_delta = None
    smoothing_factor = 0.2
    speed = None
    area = None
    delta = None


    bytes_data = b''
    previous_area = previous_pos = previous_time = None
    mic_trigger = mic_analog = bpm = 0
    breath_rate = accel_magnitude = 0.0
    erregungswert = 0
    addr = None
    last_udp_process_time = last_stream_retry = 0
    xold = yold = zold = 0

    try:
        while True:
            current_time = time.time()

            # === UDP empfangen ===
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
                            accel_magnitude = calculate_accel_magnitude(x, y, z, xold, yold, zold)
                            xold, yold, zold = x, y, z
                        last_udp_process_time = current_time
            except socket.timeout:
                pass

            # === Stream lesen ===
            if stream:
                try:
                    bytes_data += stream.read(8192)
                except Exception as e:
                    try:
                        stream.close()
                    except Exception as close_err:
                        print(f"Fehler beim Schließen des Streams: {close_err}")
                    stream = None
                    print(f"Stream-Verbindung verloren: {e}")
            elif time.time() - last_stream_retry > 3:
                stream = open_stream()
                last_stream_retry = time.time()

            a = bytes_data.find(b'\xff\xd8')
            b = bytes_data.find(b'\xff\xd9')

            if a != -1 and b != -1 and a < b:
                jpg = bytes_data[a:b + 2]
                bytes_data = bytes_data[b + 2:]

                frame = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)

                if frame is None:
                    continue
                frame = cv2.flip(frame, -1)
                #entzerren
                #frame = undistort_fisheye(frame, strength=0.08)


                # --- Pupillenerkennung nur in bestimmten Zellen ---
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                eyes = eye_cascade.detectMultiScale(gray, 1.3, 5)

                filtered_eyes = []
                for (x, y, w, h) in eyes:
                    if is_in_allowed_cells(x, y, w, h):
                        filtered_eyes.append((x, y, w, h))

                for (x, y, w, h) in filtered_eyes:
                    cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

                # -- Raster zeichnen
                draw_grid(frame, GRID_SPACING, ALLOWED_CELLS)

                # Geschwindigkeit, Fläche, Delta berechnen, falls vorhanden
                if previous_pos is not None and previous_time is not None:
                    dt = current_time - previous_time
                    if dt > 0:
                        current_pos = np.mean([x + w / 2 for (x, y, w, h) in filtered_eyes]) if filtered_eyes else None
                        if current_pos is not None:
                            speed = (current_pos - previous_pos) / dt
                        else:
                            speed = 0
                else:
                    speed = 0

                area = sum([w * h for (x, y, w, h) in filtered_eyes])
                delta = (area - previous_area) if previous_area is not None else 0

                previous_area = area
                previous_pos = np.mean([x + w / 2 for (x, y, w, h) in filtered_eyes]) if filtered_eyes else None
                previous_time = current_time

                # Nach Berechnung von speed, area, delta:

            if smoothed_speed is None:
                smoothed_speed = speed
            else:
                smoothed_speed = smoothing_factor * speed + (1 - smoothing_factor) * smoothed_speed

            if smoothed_area is None:
                smoothed_area = area
            else:
                smoothed_area = smoothing_factor * area + (1 - smoothing_factor) * smoothed_area

            if smoothed_delta is None:
                smoothed_delta = delta
            else:
                smoothed_delta = smoothing_factor * delta + (1 - smoothing_factor) * smoothed_delta

                print(f"Smoothed - Area: {smoothed_area}, Smoothed Speed: {smoothed_speed if 'speed' in locals() else 'N/A'}, Smoothed Delta: {smoothed_delta}")

                erregungswert = calculate_erregungswert(smoothed_speed, smoothed_area, smoothed_delta, breath_rate, accel_magnitude, bpm)

                # Glätten
                if smoothed_erregungswert is None:
                    smoothed_erregungswert = erregungswert
                else:
                    smoothed_erregungswert = (smoothing_factor * erregungswert +
                                             (1 - smoothing_factor) * smoothed_erregungswert)
                    
                #zurücksenden

                if addr is not None:
                 response = f"Erregung:{int(smoothed_erregungswert)}"
                 sock.sendto(response.encode(), addr)


                # LED Werte als Text auf Bild
                cv2.putText(frame, f"Erregungswert: {int(smoothed_erregungswert)}", (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
                cv2.putText(frame, f"BPM: {bpm} MicTrigger: {mic_trigger} MicAnalog: {mic_analog}", (10, 60),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 1)
                cv2.putText(frame, f"BreathRate: {breath_rate:.1f} AccelMag: {accel_magnitude:.2f}", (10, 90),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 1)

                cv2.imshow('Pupillenerkennung (nur in bestimmten Zellen)', frame)

                key = cv2.waitKey(1)
                if key == 27:  # ESC Taste
                    break

    except KeyboardInterrupt:
        print("Beendet durch Benutzer")

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
