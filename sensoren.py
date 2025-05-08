import socket
import time

# Erstelle ein UDP-Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 4210))  # Port anpassen, wenn nötig

print("Warten auf UDP-Daten...")

last_received = time.time()  # Variable, um die letzte Empfangszeit zu verfolgen

while True:
    # Empfange Daten vom UDP-Socket (max. 1024 Bytes)
    sock.settimeout(1.0)  # Setze ein Timeout von 1 Sekunde für den Empfang
    try:
        data, addr = sock.recvfrom(1024)  # Puffergröße von 1024 Bytes
        print(f"Empfangen von {addr}: {data.decode()}")
        last_received = time.time()  # Aktualisiere die letzte Empfangszeit
    except socket.timeout:
        # Wenn kein Datenempfang in der letzten Sekunde
        print("Keine Daten empfangen in der letzten Sekunde. UDP-Verbindung ist aktiv.")
    
    # Überprüfe alle 1 Sekunde, ob die Verbindung noch aktiv ist
    if time.time() - last_received >= 1:
        print("Die Verbindung ist aktiv, aber keine neuen Daten empfangen.")
        last_received = time.time()  # Setze den Timer zurück, um regelmäßig zu prüfen
