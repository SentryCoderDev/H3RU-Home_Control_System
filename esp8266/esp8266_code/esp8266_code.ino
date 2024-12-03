#include <Wire.h>
#include <Adafruit_MS_PWM_Synth.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* serverIP = "http://your_server_ip:8000";

Adafruit_MS_PWM_Synth synth;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
  
  synth.begin();
}

void loop() {
  HTTPClient http;
  
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(serverIP + String("/result"));
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      if (payload.startsWith("{\"result\":\"")) {
        String message = payload.substring(11, payload.length() - 2);  // Parsing result
        playAudioMessage(message);
      }
    }
    http.end();
  }
  delay(10000); // Poll every 10 seconds
}

void playAudioMessage(String message) {
  String audioFileName = "/static/audio/" + message + ".wav"; // Construct file path
  
  // Play audio file (Implement your audio file playing logic here)
  Serial.println("Playing: " + audioFileName);
}
