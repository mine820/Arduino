#include <WiFiClientSecure.h>

// WiFi情報
#define WIFI_SSID "wifi_ssid"
#define WIFI_PSK "wifi_pass"

// Host情報
#define DEST_HOST "weather.yahoo.co.jp"
#define DEST_URL "https://weather.yahoo.co.jp/weather/jp/12/4510/12203.html"

// 証明書（Yahooのルート）
const char* test_root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure client;

// PIN情報
#define BUTTON_PIN 4    // the number of the pushbutton pin
#define RED_LED_PIN 2   // the number of the LED Red pin
#define BLUE_LED_PIN 15 // the number of the LED Blue pin

//*******************************************************************
void setup() {
  Serial.begin(115200);
  
  // PINの初期化
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  // WiFi接続
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.println(WiFi.localIP());
}

//*******************************************************************
String getContent()
{
  String buffer = "";

  // 証明書を設定
  Serial.println("\nStarting connection to server...");
  client.setCACert(test_root_ca);

  // Hostに接続
  if (!client.connect(DEST_HOST, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    String str1 = String("GET ") + String(DEST_URL) + " HTTP/1.1\r\n";
    str1 += "Host: " + String(DEST_HOST) + "\r\n";
    str1 += "User-Agent: BuildFailureDetectorESP32\r\n";
    str1 += "Connection: close\r\n\r\n"; //closeを使うと、サーバーの応答後に切断される。最後に空行必要
    str1 += "\0";
    client.print(str1);
    client.flush();

    // HTMLの取得
    while (client.connected()) {
      // ヘッダは不要
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    while (client.available()) {
      // 1行ごとに保存
      String line = client.readStringUntil('\r');
      buffer += line;
    }

    // 文字化け対応
    buffer += "\0";
    buffer.replace("&amp;","&");    // XMLソースの場合、半角&が正しく表示されないので、全角に置き換える
    buffer.replace("&#039;","\'");  // XMLソースの場合、半角アポストロフィーが正しく表示されないので置き換える
    buffer.replace("&#39;","\'");   // XMLソースの場合、半角アポストロフィーが正しく表示されないので置き換える
    buffer.replace("&apos;","\'");  // XMLソースの場合、半角アポストロフィーが正しく表示されないので置き換える
    buffer.replace("&quot;","\"");  // XMLソースの場合、ダブルクォーテーションが正しく表示されないので置き換える

    // 切断
    client.stop();
  }

  return buffer;
}

//*******************************************************************
String getWeathrString(String buffer)
{
  // HTMLをスクレイピング
  int start_index = 0;
  int end_index = 0;
  
  // 最初のyjw_table2の中の
  start_index = buffer.indexOf("class=\"yjw_table2\"", start_index);
  
  // 18番目の<small>から
  for (int i=0; i < 18; i++){
    start_index++;
    start_index = buffer.indexOf("<small>", start_index);
  }
  // 次の</small>まで
  end_index = buffer.indexOf("</small>", start_index);

  // 文字列切り出し
  return buffer.substring(start_index+7, end_index);
}

//*******************************************************************
int checkWeather()
{
  // HTMLの取得
  String buffer = getContent();
  //Serial.println(buffer);

  // 天気の取得
  int number = 1;
  String weather = getWeathrString(buffer);
  Serial.println(weather);
  if (weather.indexOf("雨") >= 0){
    number = 0;
  }
  return number;
}

//*******************************************************************
void loop() {
  // ボタンの状態を取得
  int buttonState = digitalRead(BUTTON_PIN);

  // 押されていた時の処理
  if (buttonState == HIGH) {
    // 天気予報を確認
    switch (checkWeather()){
      case 0: // 雨
        // 赤LEDを点灯
        Serial.println("Red!");
        digitalWrite(RED_LED_PIN, HIGH);
        delay(1000); 
        digitalWrite(RED_LED_PIN, LOW);
        break;
      case 1: // 雨以外
        // 青LEDを点灯
        Serial.println("Blue!");
        digitalWrite(BLUE_LED_PIN, HIGH);
        delay(1000); 
        digitalWrite(BLUE_LED_PIN, LOW);
        break;
    }
  }
}
