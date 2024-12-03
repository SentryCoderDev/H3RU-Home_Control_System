### Project Name

**H3RU Home Control System**

### Description

This project is a system developed to provide security and automation at home using NVIDIA Jetson Nano and Arduino. The system integrates various cameras and provides control of the garage and entrance door through a web interface. Users can open the door via the web, receive doorbell notifications, and engage in voice communication.

### Features

- **Web Control Interface**:
  - Control garage and entrance door through a web dashboard.
  - Real-time camera feeds accessible via the web.
  
- **Doorbell Integration**:
  - Receive doorbell notifications on the web interface.
  - Play custom doorbell sounds when triggered.

- **Voice Communication**:
  - Two-way voice communication through the web interface.
  
- **3 Camera Support**:
  - Camera monitoring the garage exterior.
  - Camera monitoring the garage interior.
  - Camera monitoring the entrance.

- **Personalized Messages**: Displays personalized greeting messages for each user on the LCD screen.

### Hardware Requirements

- NVIDIA Jetson Nano
- Arduino (UNO or Mega)
- 4x3 Keypad (I created a button circuit that includes a common GND connection line that I created myself.)
- 3 USB Cameras
- Ethernet Cable or Wi-Fi Module
- 16x2 LCD Screen
- Doorbell Button
- Speaker or Audio Output Device

### Software Requirements

- Python 3
- FastAPI (web framework)
- Uvicorn (ASGI server)
- OpenCV (for image processing)
- PySerial (for serial communication)
- Jinja2 (templating engine)
- Arduino IDE

### Setup Instructions

1. **Clone the Repository**

   ```bash
   git clone https://github.com/SentryCoderDev/H3RU-Home_Control_System.git
   cd H3RU-Home_Control_System
   ```

2. **Install Python Dependencies**

   Install the required Python packages using the `requirements.txt` file:

   ```bash
   pip install -r requirements.txt
   ```

3. **Generate a Self-Signed SSL Certificate**

   The application uses secure WebSockets and requires SSL certificates. Generate a self-signed SSL certificate:

   ```bash
   openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout key.pem -out cert.pem
   ```

   Follow the prompts to enter the required information.

4. **Set Up Arduino**

   - Install the Arduino IDE.
   - Connect the Arduino to your computer.
   - Open the Arduino sketch at `/arduino/arduino_code/arduino_code.ino`.
   - Upload the code to your Arduino board.
   - Ensure all hardware components are properly connected.

5. **Run the Application**

   Start the FastAPI application using Uvicorn with SSL:

   ```bash
   uvicorn app:app --host 0.0.0.0 --port 8001 --ssl-certfile=cert.pem --ssl-keyfile=key.pem
   ```

6. **Access the Web Interface**

   Open your web browser and navigate to:

   ```
   https://<JetsonNano_IP>:8001/
   ```

   Replace `<JetsonNano_IP>` with the IP address of your Jetson Nano.

   **Note**: Since we're using a self-signed certificate, your browser may warn you about an insecure connection. You can proceed by accepting the risk and continuing to the site.

### Directory Structure

```
H3RU-Home_Control_System/
├── app.py
├── daemon.py
├── templates/
│   └── index.html
├── static/
│   ├── styles.css
│   ├── sounds/
├── arduino/
│   └── arduino_code/
│       └── arduino_code.ino
├── requirements.txt
└── README.md
```

### Contributing

If you would like to contribute, please create a pull request or open an issue.

### Licence

This project is licensed under the GNU license.
