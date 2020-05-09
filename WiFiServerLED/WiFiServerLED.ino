#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// WiFi情報
const char* ssid = "wifi_ssid";
const char* pass = "wifi_pass";

// 使用ピン
int ssr[] = {4, 5, 16, 17};
int triggerType = LOW;
int ssrON, ssrOFF;

// 初期画面
void handleRoot(void)
{
    String html;

    // HTMLを組み立てる
    html = "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset=\"utf-8\">";
    html += "<title>リレーをON/OFFする</title>";
    html += "</head>";
    html += "<body>";
    html += "<p>リンクをクリックするとリレーがON/OFFします</p>";
    html += "<ul>";
    html += "<li>リレー1を<a href=\"/relay?c=0&s=on\">ON</a>／<a href=\"/relay?c=0&s=off\">OFF</a>にする</li>";
    html += "<li>リレー2を<a href=\"/relay?c=1&s=on\">ON</a>／<a href=\"/relay?c=1&s=off\">OFF</a>にする</li>";
    html += "<li>リレー3を<a href=\"/relay?c=2&s=on\">ON</a>／<a href=\"/relay?c=2&s=off\">OFF</a>にする</li>";
    html += "<li>リレー4を<a href=\"/relay?c=3&s=on\">ON</a>／<a href=\"/relay?c=3&s=off\">OFF</a>にする</li>";
    html += "</ul>";
    html += "</body>";
    html += "</html>";

    // HTMLを出力する
    server.send(200, "text/html", html);
}

// リレーのON/OFF
void handleRelay(void)
{
    String msg;
    int c = -1;
    int s = -1;

    // 「/relay?c=○&s=□」のパラメータが指定されているかどうかを確認
    if (server.hasArg("c") && server.hasArg("s"))
    {
        // 「○」の値に応じて、リレーのチャンネルを設定する
        msg = "リレー";
        if (server.arg("c").equals("0"))
        {
            c = 0;
            msg += "1";
        }
        else if (server.arg("c").equals("1"))
        {
            c = 1;
            msg += "2";
        }
        else if (server.arg("c").equals("2"))
        {
            c = 2;
            msg += "3";
        }
        else if (server.arg("c").equals("3"))
        {
            c = 3;
            msg += "4";
        }
        msg += "を";

        // 「□」の値に応じて、リレーのON/OFFを設定する
        if (server.arg("s").equals("on"))
        {
            s = ssrON;
            msg += "ON";
        }
        else if (server.arg("s").equals("off"))
        {
            s = ssrOFF;
            msg += "OFF";
        }
        msg += "にしました";

        // 設定
        if ((c >= 0) && (s >= 0))
        {
            digitalWrite(ssr[c], s);
        }
        else
        {
            msg = "パラメータが正しく指定されていません";
        }
   }

    // 変数msgの文字列を送信する
    server.send(200, "text/plain; charset=utf-8", msg);
}

// 存在しないアドレスが指定された時の処理
void handleNotFound(void)
{
    server.send(404, "text/plain", "Not Found.");
}

// 初期化
void setup()
{
    int i;
    
    // シリアルポートの初期化
    Serial.begin(115200);

    // WiFiのアクセスポイントに接続
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    // ESP32のIPアドレスを出力
    Serial.println("WiFi Connected.");
    Serial.print("IP = ");
    Serial.println(WiFi.localIP());
    
    // Groveセンサーの初期化
    if (triggerType)
    {
        ssrON = HIGH;
        ssrOFF = LOW;
    }
    else
    {
        ssrON = LOW;
        ssrOFF = HIGH;
    }
    for (i=0; i < 4; i++)
    {
        pinMode(ssr[i], OUTPUT);
        digitalWrite(ssr[i], ssrOFF);
    }

    // 処理するアドレスを定義
    server.on("/", handleRoot);
    server.on("/relay", handleRelay);
    server.onNotFound(handleNotFound);
    // Webサーバーを起動
    server.begin();
}

// 処理ループ
void loop()
{
    server.handleClient();
}
