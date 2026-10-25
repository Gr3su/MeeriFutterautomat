import serialUtils
from flask import Flask
import time
import serial

def initSerial():
    serialComPort = serialUtils.findSendingESP()
    while not serialComPort:
        print("No ESP found. Retrying in 3 seconds...")
        time.sleep(3)
        serialComPort = serialUtils.findSendingESP()
    return serial.Serial(serialComPort, 9600, timeout=1)

app = Flask(__name__)

@app.route('/rotate/<int:id>/<int:index>/<int:steps>', methods=['GET'])
def rotate(id, index, steps):
    ser = initSerial()

    if not ser or not ser.is_open:
        return f"{id}, 0, 0, Error: No serial connection, 4", 500

    try:
        command = f"{id}, 2, {index}, {steps}, 0\n"
        ser.write(command.encode('utf-8'))
        ser.timeout = 180
        response = ser.readline().decode('utf-8').strip()
        ser.timeout = 1
        return response
    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500

@app.route('/home/<int:id>/<int:index>', methods=['GET'])
def home(id, index):
    ser = initSerial()

    if not ser or not ser.is_open:
        return f"{id}, 0, 0, Error: No serial connection, 4", 500

    try:
        command = f"{id}, 3, {index}, 0, 0\n"
        ser.write(command.encode('utf-8'))
        ser.timeout = 180
        response = ser.readline().decode('utf-8').strip()
        ser.timeout = 1
        return response
    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500

@app.route('/home_all/<int:id>', methods=['GET'])
def home_all(id):
    ser = initSerial()

    if not ser or not ser.is_open:
        return f"{id}, 0, 0, Error: No serial connection, 4", 500

    try:
        command = f"{id}, 4, 0, 0, 0\n"
        ser.write(command.encode('utf-8'))
        ser.timeout = 180
        response = ser.readline().decode('utf-8').strip()
        ser.timeout = 1
        return response
    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500

@app.route('/reset/<int:id>/<int:index>', methods=['GET'])
def reset(id, index):
    ser = initSerial()

    if not ser or not ser.is_open:
        return f"{id}, 0, 0, Error: No serial connection, 4", 500

    try:
        command = f"{id}, 5, {index}, 0, 0\n"
        ser.write(command.encode('utf-8'))
        ser.timeout = 180
        response = ser.readline().decode('utf-8').strip()
        ser.timeout = 1
        return response
    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500

@app.route('/reset_all/<int:id>', methods=['GET'])
def reset_all(id):
    ser = initSerial()

    if not ser or not ser.is_open:
        return f"{id}, 0, 0, Error: No serial connection, 4", 500

    try:
        command = f"{id}, 6, 0, 0, 0\n"
        ser.write(command.encode('utf-8'))
        ser.timeout = 180
        response = ser.readline().decode('utf-8').strip()
        ser.timeout = 1
        return response
    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500

if __name__ == '__main__':
    app.run(debug=True)