import socket
import sys

UDP_IP = "0.0.0.0"
UDP_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"UDP-Empfänger gestartet auf Port {UDP_PORT}...\n")

try:
    while True:
        data, addr = sock.recvfrom(1024)
        decoded = data.decode("utf-8", errors="ignore").strip()

        output = f"[{addr[0]}] Empfangen: {decoded}"
        if "X:" in decoded and "MicTrigger:" in decoded:
            try:
                parts = decoded.split(",")
                x = int(parts[0].split(":")[1])
                y = int(parts[1].split(":")[1])
                z = int(parts[2].split(":")[1])
                mic_trigger = int(parts[3].split(":")[1])

                bpm = "?"     # Standardwert, falls nicht gefunden
                avg_bpm = "?" # Standardwert für AvgBPM

                for part in parts:
                    if "BPM:" in part and "AvgBPM" not in part:
                        bpm = int(part.split(":")[1])
                    if "AvgBPM:" in part:
                        avg_bpm = int(part.split(":")[1])

                output = (
                    f"X={x:>5} | "
                    f"Y={y:>5} | "
                    f"Z={z:>5} | "
                    f"MIC={mic_trigger:>2} | "
                    f"BPM={bpm:>3} | "
                    f"AvgBPM={avg_bpm:>3}"
                )

            except Exception as e:
                output = f"⚠️  Fehler beim Parsen: {e}"

        print(f"\r{output:<120}", end="")  # etwas mehr Platz
        sys.stdout.flush()

except KeyboardInterrupt:
    print("\nManuell beendet.")
finally:
    sock.close()
    print("Socket geschlossen.")
