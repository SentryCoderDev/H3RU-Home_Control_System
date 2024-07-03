#include <RCSwitch.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <SPI.h>

// RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

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

// Kullanıcı RFID kartları ve şifreler
String validCards[] = {"CARD1_UID", "CARD2_UID"};
String userPasswords[] = {"1234", "5678"};
String userNames[] = {"X", "Y"};

String enteredPassword = "";

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
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
  // Solenoidi açma kodu burada olacak
  // Örneğin: digitalWrite(solenoidPin, HIGH);
}

void sendResultToJetson(String message) {
  Serial.print("RESULT:");
  Serial.println(message);
}
