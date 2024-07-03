### Project name

**H3RU Home Control System**

### Explanation

This project is a system developed to provide security and automation at home using NVIDIA Jetson Nano and Arduino. The system works integrated with various cameras and a solenoid lock mechanism and provides control of the garage and entrance door. Users can open the door with their RFID card or keypad and specific messages are displayed for each user.

### Features

- **3 Camera Support**:
 - Camera that detects motion and recognizes license plates outside the garage (10 meters away).
 - Camera detecting vehicle entry inside the garage.
 - Camera at the entrance door that detects ringtones and provides video streaming.
- **RFID and Keypad Login**: Users can log in with their RFID cards or password.
- **Personal Messages**: Personal greeting messages for each user.
- **Control via Internet**: Possibility of control over the internet by connecting to Jetson Nano via Ethernet cable.
- **Displaying Results**: Results from Arduino are displayed in the web interface via Jetson Nano.

### Required Hardware

- NVIDIA Jetson Nano
- Arduino (UNO or Mega)
- MFRC522 RFID Reader
- RF transreceiver
- 4x3 Keypad 
- RCSwitch library (RF control for garage door)
- 3 USB Cameras
- Ethernet Cable
- Solenoid lock
- 16x2 screen

### Required Software

-Python 3
- Flask (web framework)
- OpenCV (for image processing)
- Arduino IDE

### Setup

1. **Jetson Nano**:
 - Install Jetson Nano and install necessary software.
 - Clone project files.
 - Run the Flask application.

2. **Arduino**:
 - Install Arduino and install the necessary libraries.
 - Upload the Arduino code.

### Use

1. **Launch Flask Application**:
 ```bash
 python app.py
 ```

2. **Access the Web Interface**:
 Go to `http://<JetsonNano_IP>:5000` from your browser.

3. **Watch Footage**:
 You can view images from three cameras in the web interface.

4. **Open the Door**:
 Scan your RFID card or enter your password with Keypad.

5. **View Result Messages**:
 You can see the login process results in the web interface.

### Directory Structure

```
HomeControlSystem/
├── app.py
├── templates/
│ └── index.html
├──static/
│ └── styles.css
└── arduino/
 └── arduino_code.ino
```

### Contributing

If you would like to contribute, please create a pull request or open an issue.

### Licence

This project is licensed under the GNU license.
