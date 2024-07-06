#include <RCSwitch.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// RFID
#define SS_PIN 6
#define RST_PIN 5
#define solenoidPin 4
MFRC522 rfid(SS_PIN, RST_PIN);

const int rs = 12; // Digital Pin 12
const int en = 11; // Digital Pin 11
const int d6 = 10; // Digital Pin 10
const int d4 = 9; // Digital Pin 9
const int d5 = 8; // Digital Pin 8
const int d7 = 7; // Digital Pin 7

// Keypad
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Users RFID PASSWORD NAME
String validCards[] = {"CARD1_UID", "CARD2_UID"};
String userPasswords[] = {"1234", "5678"};
String userNames[] = {"X", "Y"};

// LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String enteredPassword = "";

void setup() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
}

void loop() {
    // process incoming commands from serial port
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        if (command == "OPEN_DOOR") {
            openDoor();
        }
    }

    // RFID Read
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        String cardUID = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            cardUID += String(rfid.uid.uidByte[i], HEX);
        }
        if (isValidCard(cardUID)) {
            String user = getUserByCard(cardUID);
            openDoor();
            displayGreeting(user, cardUID);
            Serial.print("Hello ");
            Serial.println(user);
            sendResultToJetson("Hello " + user);
        }
        rfid.PICC_HaltA();
    }

    // Keypad Read
    char key = keypad.getKey();
    if (key) {
        if (key == '#') {
            if (isValidPassword(enteredPassword)) {
                String user = getUserByPassword(enteredPassword);
                openDoor();
                displayGreeting(user, "PASSWORD");
                Serial.print("Hello ");
                Serial.println(user);
                sendResultToJetson("Hello " + user);
            }
            enteredPassword = "";
        } else {
            enteredPassword += key;
        }
    }
}

bool isValidCard(String uid) {
    for (String validCard : validCards) {
        if (validCard == uid) {
            return true;
        }
    }
    return false;
}

bool isValidPassword(String password) {
    for (String userPassword : userPasswords) {
        if (userPassword == password) {
            return true;
        }
    }
    return false;
}

String getUserByCard(String uid) {
    for (int i = 0; i < sizeof(validCards) / sizeof(validCards[0]); i++) {
        if (validCards[i] == uid) {
            return userNames[i];
        }
    }
    return "Unknown";
}

String getUserByPassword(String password) {
    for (int i = 0; i < sizeof(userPasswords) / sizeof(userPasswords[0]); i++) {
        if (userPasswords[i] == password) {
            return userNames[i];
        }
    }
    return "Unknown";
}

void openDoor() {
    digitalWrite(solenoidPin, HIGH);
    delay(1000); // 1 second delay
    digitalWrite(solenoidPin, LOW);
}

void sendResultToJetson(String message) {
    Serial.print("RESULT:");
    Serial.println(message);
}

void displayGreeting(String user, String identifier) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Merhaba ");
    lcd.print(user);
    lcd.setCursor(0, 1);
    lcd.print(identifier);
}
