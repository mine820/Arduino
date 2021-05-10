#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>

//#define UART
#ifdef UART
#include <MHZ19_uart.h>
#else
#include <MHZ19.h>
#endif

// WiFi設定情報（自分の無線LAN環境）
const char* SSID = "mySSID";  // 変更すること
const char* PASSWORD = "myPASSWORD";  // 変更すること

// AWS IoT設定情報
const char* AWS_ENDPOINT = "xxx.iot.ap-northeast-1.amazonaws.com";  // 変更すること
const int   AWS_PORT     = 8883;
const char* PUB_TOPIC    = "topic/fromDevice";
const char* SUB_TOPIC    = "topic/fromCloud";
const char* CLIENT_ID    = "esp32";

const char* ROOT_CA = "-----BEGIN CERTIFICATE-----\n" \
"****************************************************************\n" \
"-----END CERTIFICATE-----\n";  // 変更すること

const char* CERTIFICATE = "-----BEGIN CERTIFICATE-----\n" \
"****************************************************************\n" \
"-----END CERTIFICATE-----\n";  // 変更すること

const char* PRIVATE_KEY = "-----BEGIN RSA PRIVATE KEY-----\n" \
"****************************************************************\n" \
"-----END RSA PRIVATE KEY-----\n";  // 変更すること

// MQTT設定
#define QOS 0 // 届こうが届くまいが1回だけメッセージをPublishする（0か1を指定可）
WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);

// MH-Z19C
#ifdef UART
MHZ19_uart myMHZ19;
const int rx_pin = 16; //Serial rx pin no
const int tx_pin = 17; //Serial tx pin no
#else
const int pwm_pin = 14;
MHZ19* myMHZ19 = new MHZ19(pwm_pin);
#endif
char pubMessage[128];

void setup_wifi(){
  Serial.print("Connecting to ");
  Serial.println(SSID);

  // ESP32でWiFiに繋がらなくなるときのための対策
  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_awsiot(){
  httpsClient.setCACert(ROOT_CA);
  httpsClient.setCertificate(CERTIFICATE);
  httpsClient.setPrivateKey(PRIVATE_KEY);
  mqttClient.setServer(AWS_ENDPOINT, AWS_PORT);
  mqttClient.setCallback(mqttCallback);
}

void setup_mhz19(){
#ifdef UART
  myMHZ19.begin(rx_pin, tx_pin);
  myMHZ19.setAutoCalibration(false);
  while( myMHZ19.isWarming() ) {
    Serial.print("MH-Z19 now warming up...  status:");Serial.println(myMHZ19.getStatus());
    delay(1000);
  }
#else
  myMHZ19->setAutoCalibration(false);
#endif
}

void connect_awsiot() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(CLIENT_ID)) {
      Serial.println("Connected.");
      mqttClient.subscribe(SUB_TOPIC, QOS);
      Serial.println("Subscribed.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Try again in 5 seconds");
      // 5秒後にリトライ
      delay(5000);
    }
  }
}

void mqttCallback (char* topic, byte* payload, unsigned int length) {
    Serial.print("Received. topic=");
    Serial.println(topic);

    char subMessage[length];
    for (int i=0; i < length; i++) {
      subMessage[i] = (char)payload[i];
    }
    Serial.println(subMessage);

    // JSON前提でパースする
    JSONVar obj = JSON.parse(subMessage);
    // 今は受け取っても何も処理しない
}

void setup(){
  Serial.begin(115200);
  delay(100);
  
  setup_wifi();
  setup_awsiot();
  setup_mhz19();
}

void loop(){
  // CO2取得
#ifdef UART
  int CO2 = myMHZ19.getPPM();
#else
  int CO2 = myMHZ19->getPpmPwm();
#endif

  // Publishするメッセージの作成
  sprintf(pubMessage, "{\"co2\": %d}", CO2);
  Serial.print("Publishing message to topic ");
  Serial.println(PUB_TOPIC);
  Serial.println(pubMessage);

  mqttClient.loop();
  while (!mqttClient.publish(PUB_TOPIC, pubMessage)) {
    if (!mqttClient.connected()) {
      connect_awsiot();
    }
    mqttClient.loop();
  }
  Serial.println("Published.");

  delay(1000*60*5); // 5min
//  delay(1000*5); // 5sec
}
