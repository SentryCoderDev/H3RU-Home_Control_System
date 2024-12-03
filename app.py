import logging
import asyncio
from datetime import datetime
from fastapi import FastAPI, WebSocket, Request, HTTPException, WebSocketDisconnect
from fastapi.responses import HTMLResponse, StreamingResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from contextlib import asynccontextmanager
from pydantic import BaseModel
import cv2
import threading
import serial
import os
import glob
from typing import List
import random
import base64
import json

# Sabit Değişkenler
CAMERAS = []
RESULT_MESSAGE = ""
CONNECTED_CLIENTS = set()

# Kilit Mekanizması
lock = asyncio.Lock()

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup code
    asyncio.create_task(read_from_arduino())
    start_cameras()
    # Remove the continuous broadcast task
    # asyncio.create_task(broadcast_sounds())
    yield
    # Shutdown code (if any)

# Create FastAPI app with lifespan
app = FastAPI(lifespan=lifespan)

templates = Jinja2Templates(directory="/home/jetson/Desktop/H3RU/templates")
app.mount("/static", StaticFiles(directory="/home/jetson/Desktop/H3RU/static"), name="static")

# Log Dosyası Ayarları
logging.basicConfig(filename='access_log.txt', level=logging.INFO, format='%(asctime)s - %(message)s')

# Sound directory for doorbell sounds
SOUNDS_DIR = os.path.join("/home/jetson/Desktop/H3RU/static", "sounds")

# Global list to keep track of active WebSocket connections
active_connections: List[WebSocket] = []

# Global list to keep track of active voice WebSocket connections
voice_connections: List[WebSocket] = []

# Kullanıcı Şifreleri
USER_PASSWORDS = {"user1": "1234", "user2": "5678"}

# Seri Portu Bulma
def find_arduino_port():
    ports = glob.glob('/dev/ttyUSB*')
    if ports:
        return ports[0]
    else:
        raise RuntimeError("Arduino seri portu bulunamadı")

# Arduino Bağlantısı
try:
    SER = serial.Serial(find_arduino_port(), 115200, timeout=1)
except Exception as e:
    logging.error(f"Arduino bağlantısı başarısız: {e}")
    SER = None

# Arduino Mesaj Okuma
async def read_from_arduino():
    """Arduino'dan veri okur."""
    global RESULT_MESSAGE
    while True:
        if SER and SER.in_waiting > 0:
            try:
                line = SER.readline().decode('utf-8').strip()
                logging.info(f"Arduino Mesajı: {line}")
                print(f"Arduino Mesajı: {line}")  # Print to FastAPI terminal
                if line.startswith("RESULT:"):
                    async with lock:
                        RESULT_MESSAGE = line.split("RESULT:")[1]
                    await broadcast_message(f"Jetson: {RESULT_MESSAGE}")
                if line == "doorbell":
                    await broadcast_message("doorbell")  # Trigger the doorbell sound
                    # Remove the random sound broadcast
                    # await broadcast_random_sound()
            except Exception as e:
                logging.error(f"Arduino okuma hatası: {e}")
                print(f"Arduino okuma hatası: {e}")  # Print error to FastAPI terminal
        await asyncio.sleep(0.1)

# WebSocket Mesaj Yayını
async def broadcast_message(message: str):
    """Tüm WebSocket istemcilerine mesaj gönderir."""
    for connection in active_connections:
        await connection.send_text(message)
    for connection in voice_connections:
        if message == "doorbell":
            await connection.send_text("doorbell")

async def broadcast_random_sound():
    sound_files = [f for f in os.listdir(SOUNDS_DIR) if f.endswith(('.wav', '.mp3'))]
    if sound_files and active_connections:
        sound_file = random.choice(sound_files)
        sound_url = f"/static/sounds/{sound_file}"
        print(f"Broadcasting sound: {sound_url}")
        disconnected = []
        for connection in active_connections:
            try:
                await connection.send_text(sound_url)
            except WebSocketDisconnect:
                disconnected.append(connection)
        for connection in disconnected:
            active_connections.remove(connection)
            print(f"Client disconnected during broadcast: {connection.client}")

# Pydantic Modeli
class User(BaseModel):
    user: str
    password: str

# Kamera Başlatma
def start_cameras():
    """Kameraları başlatır."""
    for i in range(3):  # 3 kamera denemesi yapıyoruz
        cam = cv2.VideoCapture(i)
        if cam.isOpened():
            CAMERAS.append(cam)
            logging.info(f"Kamera {i} başarıyla açıldı.")
        else:
            logging.error(f"Kamera {i} açılamadı.")

# Kamera Akışı
def gen_frames(cam_index: int):
    cam = CAMERAS[cam_index]
    while True:
        success, frame = cam.read()
        if not success:
            logging.error(f"Kamera {cam_index} akışı başarısız.")
            break
        _, buffer = cv2.imencode('.jpg', frame)
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + buffer.tobytes() + b'\r\n')

# API Rotaları
@app.post("/open_door")
async def open_door():
    if SER:
        SER.write(b'OPEN_DOOR\n')
        return {"message": "Kapı açılıyor."}
    raise HTTPException(status_code=500, detail="Arduino bağlantısı yok")

@app.post("/open_garage")
async def open_garage():
    if SER:
        SER.write(b'OPEN_GARAGE\n')
        return {"message": "Garaj kapısı açılıyor."}
    raise HTTPException(status_code=500, detail="Arduino bağlantısı yok")

@app.get("/", response_class=HTMLResponse)
async def get(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/video_feed/{cam_id}")
async def video_feed(cam_id: int):
    if 0 <= cam_id < len(CAMERAS):
        return StreamingResponse(gen_frames(cam_id), media_type="multipart/x-mixed-replace; boundary=frame")
    raise HTTPException(status_code=404, detail="Geçersiz Kamera ID")

@app.post("/validate_password")
async def validate_password(user: User):
    if USER_PASSWORDS.get(user.user) == user.password:
        SER.write(b'VALID_PASSWORD\n')
        return {"message": "Şifre doğrulandı, kapı açılıyor."}
    raise HTTPException(status_code=401, detail="Geçersiz Şifre")

@app.get("/result")
async def result():
    async with lock:
        return {"result": RESULT_MESSAGE}

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    active_connections.append(websocket)
    print(f"Client connected: {websocket.client}")
    try:
        while True:
            message = await websocket.receive_text()
            if not message.strip():
                continue
            data = json.loads(message)
            mode = data.get('mode')

            if mode == 1:
                # Mode 1: Trigger doorbell
                await broadcast_message("doorbell")
                # Removed broadcasting random sound
                # await broadcast_random_sound()
            elif mode == 2:
                # Mode 2: Play a specific sound
                sound_file = data.get('sound_file')
                if sound_file:
                    sound_url = f"/static/sounds/{sound_file}"
                    await broadcast_message(sound_url)
            elif mode == 3:
                # Mode 3: Open door
                if SER:
                    SER.write(b'OPEN_DOOR\n')
                    await websocket.send_text("Door is opening.")
                else:
                    await websocket.send_text("Arduino connection unavailable.")
            elif mode == 4:
                # Mode 4: Open garage
                if SER:
                    SER.write(b'OPEN_GARAGE\n')
                    await websocket.send_text("Garage is opening.")
                else:
                    await websocket.send_text("Arduino connection unavailable.")
            # Add more modes as needed
            else:
                # Unknown mode or keep-alive message
                pass
    except ValueError:
        print("Invalid JSON message received.")
        await websocket.send_text("Invalid message format.")
    except WebSocketDisconnect:
        active_connections.remove(websocket)
        print(f"Client disconnected: {websocket.client}")

@app.websocket("/voice_ws")
async def voice_websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    voice_connections.append(websocket)
    try:
        while True:
            data = await websocket.receive_bytes()
            # Broadcast the received audio data to all other voice clients
            for connection in voice_connections:
                if connection != websocket:
                    await connection.send_bytes(data)
    except WebSocketDisconnect:
        voice_connections.remove(websocket)

# Ana Başlatıcı fonksiyonunu sadeleştirin
def main():
    loop = asyncio.get_event_loop()
    loop.create_task(read_from_arduino())
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "app:app",
        host="0.0.0.0",
        port=8001,
        ssl_certfile="cert.pem",
        ssl_keyfile="key.pem",
        ws='auto',
        http='auto'
    )
