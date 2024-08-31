#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define RelayPin 2

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo garageServo;

const int buttonPins[] = {13, 12, 11, 10, 9, 8, 7, 6, 5};
const int buttonNumbers[] = {7, 4, 1, 8, 5, 2, 9, 6, 3};

String userPasswords[] = {"1234", "5678"};
String userNames[] = {"User1", "User2"};

String enteredPassword = "";
String lastResult = "";  // To store the face recognition result

void setup() {
    Serial.begin(115200);
    Serial.println("Serial connection started. Type 'OPEN_DOOR' to test.");

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("    HVZ House   ");
    lcd.setCursor(0, 1);
    lcd.print("  Hosgeldiniz!  ");
    
    pinMode(RelayPin, OUTPUT);
    digitalWrite(RelayPin, HIGH);

    for (int i = 0; i < 9; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command == "OPEN_DOOR") {
            openDoor();
        } else if (command == "VALID_PASSWORD") {
            openDoor();
        } else if (command == "OPEN_GARAGE") {
            openGarage();           
        } else if (command == "FACE_RECOGNITION") {
            Serial.println("Facial Recognition Started...");        
        } else if (command.startsWith("IDENTIFIED:")) {
            String name = command.substring(11);  // Remove the "IDENTIFIED:" part
            displayFaceRecognitionGreeting(name);
        }
    }

    for (int i = 0; i < 9; i++) {
        if (digitalRead(buttonPins[i]) == LOW) {
            String key = String(buttonNumbers[i]);
            enteredPassword += key;
            Serial.print("Entered: ");
            Serial.println(enteredPassword);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("    HVZ House   ");
            lcd.setCursor((16 - enteredPassword.length()) / 2, 1);
            lcd.print(enteredPassword);

            if (enteredPassword.length() >= 4) {
                if (isValidPassword(enteredPassword)) {
                    String user = getUserByPassword(enteredPassword);
                    delay(1000);
                    displayGreeting(user, "Sifre");
                    Serial.print("Hello ");
                    Serial.println(user);
                    sendResultToJetson("Hello " + user);
                    openDoor();
                } else {
                    Serial.println("Invalid Password!");
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("  Hatali Sifre  ");
                    delay(2000);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("    HVZ House   ");
                }
                enteredPassword = "";
            }
            delay(200);
        }
    }
}

bool isValidPassword(String password) {
    for (String userPassword : userPasswords) {
        if (userPassword == password) {
            return true;
        }
    }
    return false;
}

String getUserByPassword(String password) {
    for (int i = 0; i < (sizeof(userPasswords) / sizeof(userPasswords[0])); i++) {
        if (userPasswords[i] == password) {
            return userNames[i];
        }
    }
    return "Unknown";
}

void openDoor() {
    Serial.println("openDoor() called");
    digitalWrite(RelayPin, LOW);
    delay(5000);
    digitalWrite(RelayPin, HIGH);
    Serial.println("DOOR OPENED");
}

void openGarage() {
    Serial.println("openGarage() called");
    garageServo.attach(4);  //Connect (activate) the servo
    garageServo.write(180);  
    Serial.println("GARAGE OPENED");
    delay(1000);             
    garageServo.write(30);   
    Serial.println("Servo Up");
    delay(500);              
    garageServo.detach();    // Disable servo

}

void sendResultToJetson(String message) {
    Serial.print("RESULT:");
    Serial.println(message);
}

void displayGreeting(String user, String identifier) {
    lcd.clear();
    lcd.setCursor((16 - (8 + user.length())) / 2, 0); // 'Merhaba' is 8 characters long.
    lcd.print("Merhaba ");
    lcd.print(user);
    
    String fullText = "Giris: " + identifier;
    lcd.setCursor((16 - fullText.length()) / 2, 1);
    lcd.print(fullText);

    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    HVZ House   ");
    lcd.setCursor(0, 1);
    lcd.print("  Hosgeldiniz!  ");
}

void displayFaceRecognitionGreeting(String name) {
    lcd.clear();
    lcd.setCursor((16 - (8 + name.length())) / 2, 0); // 'Merhaba' is 8 characters long.
    lcd.print("Merhaba ");
    lcd.print(name);

    lcd.setCursor((16 - 22) / 2, 0); //to center the phrase "the homeowner has been informed"
    lcd.print("H3RUM0'e bilgi");
    lcd.setCursor((16 - 22) / 2, 1); // Ekranda ikinci satır
    lcd.print("verildi");
    
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    HVZ House   ");
    lcd.setCursor(0, 1);
    lcd.print("  Hosgeldiniz!  ");
}
