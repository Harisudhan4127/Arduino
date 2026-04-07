import serial
import serial.tools.list_ports
import socket
import threading
import json
from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# Global serial controller
ser = None
current_config = {"port": "COM3", "baud": 115200}

def get_computer_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try: s.connect(('8.8.8.8', 1)); ip = s.getsockname()[0]
    except: ip = '127.0.0.1'
    finally: s.close()
    return ip

# --- SERIAL THREAD ---
def serial_listen():
    global ser
    while True:
        if ser and ser.is_open:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line and (line.startswith('{') or line.startswith('[')):
                    data = json.loads(line)
                    socketio.emit('update_all', {**data, 'source': 'Wired'})
            except: pass
        threading.Event().wait(0.01)

@app.route('/update', methods=['POST'])
def wifi_update():
    data = request.json
    socketio.emit('update_all', {**data, 'source': 'Wireless', 'ip': request.remote_addr})
    return jsonify({"status": "ok"}), 200

@app.route('/config', methods=['POST'])
def update_config():
    global ser, current_config
    config = request.json
    try:
        if ser and ser.is_open: ser.close()
        current_config = config
        ser = serial.Serial(config['port'], int(config['baud']), timeout=0.1)
        return jsonify({"status": "Connected"})
    except Exception as e:
        return jsonify({"status": "Error", "msg": str(e)}), 400

@app.route('/')
def index():
    ports = [p.device for p in serial.tools.list_ports.comports()]
    return render_template('index.html', ip=get_computer_ip(), ports=ports)

if __name__ == '__main__':
    threading.Thread(target=serial_listen, daemon=True).start()
    socketio.run(app, host='0.0.0.0', port=5000)