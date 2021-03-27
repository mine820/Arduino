#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

// ディスプレイ変数の宣言
Adafruit_SSD1306 display(-1);

void setup() {
  // ディスプレイの初期化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {
  // ディスプレイをクリア
  display.clearDisplay();

  // 出力する文字の大きさ
  display.setTextSize(1);
  // 出力する文字の色
  display.setTextColor(WHITE);
  // 文字の一番端の位置
  display.setCursor(0, 0);

  // 出力する文字列
  display.println("Hello");
  display.println("World!");

  // ディスプレイへの表示
  display.display();
  // 1000msec待つ
  delay(1000);
}
