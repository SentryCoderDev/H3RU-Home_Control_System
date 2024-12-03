#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define RelayPin 2
#define DoorbellButtonPin 4  // Kapı zili butonunun bağlı olduğu pin
#define SLEEP_TIMEOUT 10000  // Ekranın kapanma süresi (milisaniye)

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int buttonPins[] = {13, 12, 11, 10, 9, 8, 7, 6, 5};
const int buttonNumbers[] = {7, 4, 1, 8, 5, 2, 9, 6, 3};

String userPasswords[] = {"5836", "3636", "1877", "1331", "5665", "7568"};
String userNames[] = {"Mehmet", "Emir", "Nebihe", "Edhem", "Konuk", "Sülo"};

String enteredPassword = "";
unsigned long lastButtonPressTime = 0;  // Son tuş basım zamanını takip eder
bool isScreenOn = true;  // Ekranın açık olup olmadığını takip eder

// Add a variable to store the last state of the doorbell button
int lastDoorbellButtonState = HIGH;

// Fonksiyon bildirimi (function declarations)
bool isValidPassword(String password);
String getUserByPassword(String password);
void openDoor();
void openGarage();
void sendResultToJetson(String message);
void displayGreeting(String user, String identifier);
void sleepScreen();
void wakeUpScreen();

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("    HVZ House   ");
    lcd.setCursor(0, 1);
    lcd.print("  Hosgeldiniz!  ");
    
    pinMode(RelayPin, OUTPUT);
    pinMode(DoorbellButtonPin, INPUT_PULLUP); 
    digitalWrite(RelayPin, HIGH);
    

    for (int i = 0; i < 9; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // Eğer belirli bir süre boyunca tuşa basılmamışsa ekranı kapat
    if (isScreenOn && currentMillis - lastButtonPressTime >= SLEEP_TIMEOUT) {
        sleepScreen();
    }

    // Sayı giriş butonlarıyla ekranı uyandırma ve şifre girişi
    bool anyButtonPressed = false;
    for (int i = 0; i < 9; i++) {
        if (digitalRead(buttonPins[i]) == LOW) {
            anyButtonPressed = true;
            if (!isScreenOn) {
                wakeUpScreen();
            }

            lastButtonPressTime = currentMillis;  // Son tuş basma zamanını güncelle
            String key = String(buttonNumbers[i]);
            enteredPassword += key;
            Serial.print("Entered: ");
            Serial.println(enteredPassword);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("    HVZ House   ");
            lcd.setCursor((16 - enteredPassword.length()) / 2, 1);
            // Şifreyi ekrana yıldız (*) olarak yazdır
            for (int j = 0; j < enteredPassword.length(); j++) {
                lcd.print("*");
            }

            if (enteredPassword.length() >= 4) {
                if (enteredPassword == "2562") {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Garaj Aciliyor");
                    Serial.println("Garaj Açılıyor");
                    delay(500);
                    openGarage();
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("  Garaj Acildi  ");
                    Serial.println("Garaj Açıldı");              
                } else if (isValidPassword(enteredPassword)) {
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

    // Ekran açık kalması gereken sürenin hesaplanması
    if (anyButtonPressed) {
        lastButtonPressTime = currentMillis;  // Ekranın kapanma süresini sıfırla
    }

    // Kapı zili butonuna basıldığında
    int doorbellButtonState = digitalRead(DoorbellButtonPin);

    if (doorbellButtonState != lastDoorbellButtonState) {
        if (doorbellButtonState == LOW) {
            // Button just pressed
            lastButtonPressTime = currentMillis;
            if (!isScreenOn) {
                wakeUpScreen();
            }
            Serial.println("doorbell");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("  Kapi Zili!   ");
            delay(2000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("    HVZ House   ");
        }
        // Save the current state for next time
        lastDoorbellButtonState = doorbellButtonState;
    }

    // Seri port üzerinden komut kontrolü
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command == "OPEN_DOOR") {
            openDoor();
        } else if (command == "OPEN_GARAGE") {
            openGarage();             
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
    Serial.println("openDoor() tetiklendi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  role1 Acildi  ");
    digitalWrite(RelayPin, LOW);
    delay(2000);
    digitalWrite(RelayPin, HIGH);
    Serial.println("röle1 Açıldı");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  role1 Kapandi ");
    
}

void openGarage() {
    Serial.println("openGarage() tetiklendi");
    
    // roleyi aktif edip servo motoruna güç ver

    delay(1000);  // Güç vermek için kısa bir gecikme
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  role2 Acildi  ");
    delay(1000);
    Serial.println("röle2 Açıldı");
    delay(2000);  // 1 saniye bekle
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  röle2 Kapandı ");
}

void sendResultToJetson(String message) {
    Serial.print("RESULT:");
    Serial.println(message);
}

void displayGreeting(String user, String identifier) {
    lcd.clear();
    lcd.setCursor((16 - (8 + user.length())) / 2, 0); // 'Merhaba ' 8 karakter uzunluğundadır.
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

void sleepScreen() {
    lcd.noBacklight();
    lcd.clear();
    isScreenOn = false;
}

void wakeUpScreen() {
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("    HVZ House   ");
    lcd.setCursor(0, 1);
    lcd.print("  Hosgeldiniz!  ");
    isScreenOn = true;
}
