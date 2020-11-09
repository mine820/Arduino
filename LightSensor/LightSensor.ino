#include <WiFi.h>

int sensorPin = A0;    // select the input pin for the potentiometer

// Wi-Fi setting
const char* ssid     = "wifi_ssid";
const char* password = "wifi_pass";

// IFTTT setting
const char* host = "maker.ifttt.com";
const char* event = "event_name";
const char* secretkey = "secret_key";

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
}

void loop() {
  // read the value from the sensor:
  int sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
  }

  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += secretkey;
  url += "?value1=";
  url += String(sensorValue);

  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
      }
  }


  // Read all the lines of the reply from server and print them to Serial
  while(client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  
  Serial.println("Deep sleep start!");
  esp_sleep_enable_timer_wakeup(300*1000*1000);
  esp_deep_sleep_start();
}
