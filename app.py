from fastapi import FastAPI, WebSocket, Request
from fastapi.responses import HTMLResponse, StreamingResponse
from fastapi.templating import Jinja2Templates
import cv2
import pytesseract
import threading
import serial
import asyncio

app = FastAPI()
templates = Jinja2Templates(directory="templates")

# All Cameras
cameras = [cv2.VideoCapture(i) for i in range(3)]
ser = serial.Serial('/dev/ttyUSB0', 115200)  # Arduino's serial port
result_message = ""

user_passwords = {
    "user1": "1234",
    "user2": "5678"
    # Add more users
}

# Pytesseract configuration
pytesseract.pytesseract.tesseract_cmd = '/usr/bin/tesseract'  # Path to Tesseract OCR executable

def gen_frames(cam_index):
    cam = cameras[cam_index]
    while True:
        if not cam.isOpened():
            cam.open(cam_index)
        success, frame = cam.read()
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

async def garage_control():
    while True:
        ret, frame = cameras[0].read()
        if ret:
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            # Perform OCR using Pytesseract
            text = pytesseract.image_to_string(gray)
            print("OCR Text:", text)
            # Check if the target plate number is recognized
            if "your_target_plate" in text:
                ser.write(b'OPEN_GARAGE\n')
                print("Garage opens for plate 'your_target_plate'")
        await asyncio.sleep(1)

async def read_from_arduino():
    global result_message
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line.startswith("RESULT:"):
                result_message = line.split("RESULT:")[1]
        await asyncio.sleep(0.1)

@app.get("/", response_class=HTMLResponse)
async def index(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/video_feed/{cam_id}")
async def video_feed(cam_id: int):
    if 0 <= cam_id < len(cameras):
        return StreamingResponse(gen_frames(cam_id), media_type="multipart/x-mixed-replace; boundary=frame")
    return "Invalid Camera ID", 404

@app.post("/open_door")
async def open_door():
    ser.write(b'OPEN_DOOR\n')
    return {"message": "Door Opened"}

@app.post("/validate_password")
async def validate_password(user: str, password: str):
    if user in user_passwords and user_passwords[user] == password:
        ser.write(b'VALID_PASSWORD\n')
        return {"message": "Password Validated"}
    return {"message": "Invalid Password"}, 401

@app.get("/result")
async def result():
    global result_message
    return {"result": result_message}

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    while True:
        data = await websocket.receive_text()
        if data == "result":
            await websocket.send_text(result_message)

if __name__ == "__main__":
    threading.Thread(target=asyncio.run, args=(garage_control(),)).start()
    threading.Thread(target=asyncio.run, args=(read_from_arduino(),)).start()
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)




