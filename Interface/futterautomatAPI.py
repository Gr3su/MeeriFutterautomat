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
    return serial.Serial(serialComPort, 9600, timeout=2)

def checkParameters(id, index = None, steps = None):
    if id < 0:
        return False
    if index is not None and (index < 0 or index > 2):
        return False
    if steps is not None and (steps < -10800 or steps > 10800):
        return False
    return True

def serialRead(ser):
    response = ser.readline().decode('utf-8').strip()
    if not response:
        raise TimeoutError("No response received from ESP within the timeout period.")
    return response

def communicateWithESP(command, id):
    ser = initSerial()

    try:
        if not ser or not ser.is_open:
            return f"{id}, 0, 0, Error: No serial connection, 4", 500
        
        ser.write(command.encode('utf-8'))
        response = serialRead(ser)
        responseId = response.split(',')[0].strip()
        responseState = response.split(',')[4].strip()
        
        if responseId != str(id) or responseState != '1':
            if 'ser' in locals() and ser.is_open:
                ser.close()
            return f"{id}, 0, 0, Error: Remote ESP did not acknowledge command, 4", 500

        ser.timeout = 180
        response = serialRead(ser)
        responseState = response.split(',')[4].strip()
        ser.timeout = 1
        return response, 200 if responseState == '3' else 500

    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return f"{id}, 0, 0, Error communicating with ESP: {e}, 4", 500
    
    finally:
        ser.timeout = 1

app = Flask(__name__)

@app.route('/rotate/<int:id>/<int:index>/<int:steps>', methods=['POST'])
def rotate(id, index, steps):
    if not checkParameters(id, index, steps):
        return f"{id}, 0, 0, Error: Invalid parameters, 4", 400
    
    command = f"{id},2,{index},{steps},0\n"
    return communicateWithESP(command, id)

@app.route('/home/<int:id>/<int:index>', methods=['POST'])
def home(id, index):
    if not checkParameters(id, index):
        return f"{id}, 0, 0, Error: Invalid parameters, 4", 400

    command = f"{id},3,{index},0,0\n"
    return communicateWithESP(command, id)

@app.route('/home_all/<int:id>', methods=['POST'])
def home_all(id):
    if not checkParameters(id):
            return f"{id}, 0, 0, Error: Invalid parameters, 4", 400

    command = f"{id},4,0,0,0\n"
    return communicateWithESP(command, id)

@app.route('/reset/<int:id>/<int:index>', methods=['POST'])
def reset(id, index):
    if not checkParameters(id, index):
        return f"{id}, 0, 0, Error: Invalid parameters, 4", 400

    command = f"{id},5,{index},0,0\n"
    return communicateWithESP(command, id)

@app.route('/reset_all/<int:id>', methods=['POST'])
def reset_all(id):
    if not checkParameters(id):
        return f"{id}, 0, 0, Error: Invalid parameters, 4", 400

    command = f"{id},6,0,0,0\n"
    return communicateWithESP(command, id)

@app.route('/change_setting/<int:id>/<int:index>/<int:value>', methods=['POST'])
def change_setting(id, index, value):
    if not checkParameters(id, index):
        return f"{id}, 0, 0, Error: Invalid parameters, 4", 400

    command = f"{id},7,{index},{value},0\n"
    return communicateWithESP(command, id)

if __name__ == '__main__':
    app.run(debug=True)