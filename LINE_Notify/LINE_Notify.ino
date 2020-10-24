#include <WiFiClientSecure.h>

#define BUTTON_PIN (4)

const char* ssid     = "my-ssid";
const char* password = "my-password";

const char* host = "notify-api.line.me";
const char* token = "my-line-notify-token";
const char* message = "Push!";

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(host, 443)){
    Serial.println("Connection failed!");
    return;
  }

  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    String query = String("message=") + String(message);
    String request = String("") +
                 "POST /api/notify HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Authorization: Bearer " + token + "\r\n" +
                 "Content-Length: " + String(query.length()) +  "\r\n" + 
                 "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
                  query + "\r\n";
    client.print(request);
  
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        break;
      }
    }
  
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
}
