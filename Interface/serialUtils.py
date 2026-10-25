import serial.tools.list_ports as listP
import serial

def findESPs():
    vid=0x1A86
    pid=0x7523

    comPorts = []

    portList = listP.comports()
    for port in portList:
        if port.vid == vid and port.pid == pid:
            comPorts.append(port.device)

    return comPorts

def findSendingESP():
    comPorts = findESPs()

    for port in comPorts:
        try:
            ser = serial.Serial(port, 9600, timeout=1)
            ser.write('pingESP\n'.encode('utf-8'))
            response = ser.readline().decode('utf-8').strip()
            if response == 'pongESP':
                return port
        except Exception as e:
            print(f"Fehler beim Überprüfen des Ports {port}: {e}")
        finally:
            if 'ser' in locals() and ser.is_open:
                ser.close()

    return None