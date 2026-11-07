import serialUtils
from flask import Flask, jsonify
from waitress import serve
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

def wrongParametersResponse(id):
    return jsonify({"id" : id, "message" : "Error: Invalid parameters", "state" : 4}, status=400, mimetype='application/json')

def serialRead(ser):
    response = ser.readline().decode('utf-8').strip()
    if not response:
        raise TimeoutError("No response received from ESP within the timeout period.")
    return response

def communicateWithESP(command, id):
    ser = initSerial()

    try:
        if not ser or not ser.is_open:
            return jsonify({"id" : id, "message" : "Error: No serial connection", "state" : 4}, status=500, mimetype='application/json')
        
        ser.write(command.encode('utf-8'))
        response = serialRead(ser)
        responseId = response.split(',')[0].strip()
        responseState = response.split(',')[4].strip()
        
        if responseId != str(id) or responseState != '1':
            if 'ser' in locals() and ser.is_open:
                ser.close()
            return jsonify({"id" : id, "message" : "Error: Remote ESP did not acknowledge command", "state" : 4}, status=500, mimetype='application/json')

        ser.timeout = 180
        response = serialRead(ser)
        responseState = response.split(',')[4].strip()
        ser.timeout = 1

        response = {"id" : id, "state" : responseState}
        if responseState != '3':
            response["message"] = "Error: Remote ESP did not complete command successfully"
            
        return jsonify(response, status=200 if responseState == '3' else 500, mimetype='application/json')

    except Exception as e:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        return jsonify({"id" : id, "message" : "Error: Problem while communicating with ESP", "state" : 4}, status=500, mimetype='application/json')
    
    finally:
        ser.timeout = 1

app = Flask(__name__)

@app.route('/rotate/<int:id>', methods=['POST'])
def test(id):
    if not checkParameters(id):
        return wrongParametersResponse(id)
    
    command = f"{id},1,0,0,0\n"
    return communicateWithESP(command, id)

@app.route('/rotate/<int:id>/<int:index>/<int:steps>', methods=['POST'])
def rotate(id, index, steps):
    if not checkParameters(id, index, steps):
        return wrongParametersResponse(id)
    
    command = f"{id},2,{index},{steps},0\n"
    return communicateWithESP(command, id)

@app.route('/home/<int:id>/<int:index>', methods=['POST'])
def home(id, index):
    if not checkParameters(id, index):
        return wrongParametersResponse(id)

    command = f"{id},3,{index},0,0\n"
    return communicateWithESP(command, id)

@app.route('/home_all/<int:id>', methods=['POST'])
def home_all(id):
    if not checkParameters(id):
            return wrongParametersResponse(id)

    command = f"{id},4,0,0,0\n"
    return communicateWithESP(command, id)

@app.route('/reset/<int:id>/<int:index>', methods=['POST'])
def reset(id, index):
    if not checkParameters(id, index):
        return wrongParametersResponse(id)

    command = f"{id},5,{index},0,0\n"
    return communicateWithESP(command, id)

@app.route('/reset_all/<int:id>', methods=['POST'])
def reset_all(id):
    if not checkParameters(id):
        return wrongParametersResponse(id)

    command = f"{id},6,0,0,0\n"
    return communicateWithESP(command, id)

@app.route('/change_setting/<int:id>/<int:index>/<int:value>', methods=['POST'])
def change_setting(id, index, value):
    if not checkParameters(id, index):
        return wrongParametersResponse(id)

    command = f"{id},7,{index},{value},0\n"
    return communicateWithESP(command, id)

if __name__ == '__main__':
    #app.run(debug=True)
    serve(app, host='0.0.0.0', port=5000, threads=1)