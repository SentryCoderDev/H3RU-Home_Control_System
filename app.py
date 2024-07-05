from flask import Flask, render_template, Response, request, jsonify
import cv2
import pytesseract
import time
import threading
import serial

app = Flask(__name__)

# All Cameras
outside_garage_cam = cv2.VideoCapture(0)  
inside_garage_cam = cv2.VideoCapture(1)   
entrance_cam = cv2.VideoCapture(2)        

ser = serial.Serial('/dev/ttyUSB0', 115200)  # Arduino's serial port
result_message = ""

user_passwords = {
    "user1": "1234",
    "user2": "5678"
    # Add more users
}

# Pytesseract configuration
pytesseract.pytesseract.tesseract_cmd = '/usr/bin/tesseract'  # Path to Tesseract OCR executable

def gen_frames(cam):
    while True:
        success, frame = cam.read()
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

def garage_control():
    while True:
        ret, frame = outside_garage_cam.read()
        if ret:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            # Perform OCR using Pytesseract
            text = pytesseract.image_to_string(gray)
            print("OCR Text:", text)
            # Check if the target plate number is recognized
            if "your_target_plate" in text:
                ser.write(b'OPEN_GARAGE\n')
                print("Opening garage for plate 'your_target_plate'")
        time.sleep(1)

def audio_detection():
    while True:
        if detect_doorbell():
            print("ringtone detected!")
            # Start camera streaming at home entrance
            # Notify and view the camera as needed
        time.sleep(1)

def detect_doorbell():
    return False

def read_from_arduino():
    global result_message
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("RESULT:"):
                result_message = line.split("RESULT:")[1]

@app.route('/video_feed/<int:cam_id>')
def video_feed(cam_id):
    if cam_id == 0:
        return Response(gen_frames(outside_garage_cam), mimetype='multipart/x-mixed-replace; boundary=frame')
    elif cam_id == 1:
        return Response(gen_frames(inside_garage_cam), mimetype='multipart/x-mixed-replace; boundary=frame')
    elif cam_id == 2:
        return Response(gen_frames(entrance_cam), mimetype='multipart/x-mixed-replace; boundary=frame')
    else:
        return "Invalid Camera ID", 404

@app.route('/open_door', methods=['POST'])
def open_door():
    ser.write(b'OPEN_DOOR\n')
    return "Door Opened", 200

@app.route('/validate_password', methods=['POST'])
def validate_password():
    data = request.json
    user = data.get('user')
    password = data.get('password')
    
    if user in user_passwords and user_passwords[user] == password:
        ser.write(b'VALID_PASSWORD\n')
        return "Password Validated", 200
    else:
        return "Invalid Password", 401

@app.route('/result')
def result():
    global result_message
    return jsonify(result=result_message)

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    threading.Thread(target=garage_control).start()
    threading.Thread(target=audio_detection).start()
    threading.Thread(target=read_from_arduino).start()
    app.run(host='0.0.0.0', port=5000, debug=True)

