from flask import Flask, render_template, Response, request
import cv2
import easyocr
import time
import threading
from your_motor_library import MotorController
from your_audio_detection_library import AudioDetector

app = Flask(__name__)

# Kameralar
outside_garage_cam = cv2.VideoCapture(0)  # 10 metre uzakta olan kamera
inside_garage_cam = cv2.VideoCapture(1)   # Garajın içindeki kamera
entrance_cam = cv2.VideoCapture(2)        # Ev girişindeki kamera

# Plaka tanıma ve motor kontrolü
target_plate = "34AEA154"
motor = MotorController()
audio_detector = AudioDetector()  # Kapı zil sesi algılama

def check_license_plate(frame, target_plate):
    reader = easyocr.Reader(['en'])
    results = reader.readtext(frame)
    for (bbox, text, prob) in results:
        if text == target_plate:
            return True
    return False

def gen_frames(cam):  # video akışı için bir jeneratör fonksiyon
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
        if ret and check_license_plate(frame, target_plate):
            motor.open_garage()
            time.sleep(2)  # Garajın açılma süresi
            inside_detection()
        time.sleep(1)

def inside_detection():
    while True:
        ret, frame = inside_garage_cam.read()
        if ret:
            # Aracın içeri girip girmediğini kontrol et
            # Burada basit bir hareket algılama yapılabilir
            # Daha gelişmiş algoritmalar kullanabilirsiniz
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            motion_detected = cv2.absdiff(gray, cv2.GaussianBlur(gray, (21, 21), 0)).sum() > 100000  # Basit bir hareket algılama
            if motion_detected:
                time.sleep(10)  # Aracın içeri girmesi için bekleme süresi
                motor.close_garage()
                break

def audio_detection():
    while True:
        if audio_detector.detect_doorbell():
            print("Zil Sesi Algılandı!")
            # Ev giriş kamera akışını başlat
            # Bu durumda kullanıcının bildirim alıp kamerayı görüntülemesi sağlanabilir
            # Bu kısımda Flask route'ları kullanarak istemci tarafına bilgi gönderebilirsiniz.
        time.sleep(1)

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
    # Solenoid kontrol kodu buraya gelecek
    motor.open_door()  # Örneğin: motor.open_door()
    return "Door Opened", 200

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    # Arka planda çalışan görevler
    threading.Thread(target=garage_control).start()
    threading.Thread(target=audio_detection).start()
    app.run(host='0.0.0.0', port=5000, debug=True)
