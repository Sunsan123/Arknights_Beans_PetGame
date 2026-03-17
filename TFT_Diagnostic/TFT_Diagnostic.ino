/*
 * TamaFi TFT_eSPI Final Test
 * Verifies TFT_eSPI works with TFT_MISO=6 fix.
 * Serial Monitor: 115200 baud.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite fb(&tft);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("=== TFT_eSPI Final Test ===\n");

  Serial.printf("TFT_MISO = %d (expect 6, NOT 17)\n", TFT_MISO);
  Serial.printf("TFT_MOSI = %d\n", TFT_MOSI);
  Serial.printf("TFT_SCLK = %d\n", TFT_SCLK);

  if (TFT_MISO == TFT_MOSI) {
    Serial.println("\n[FAIL] MISO == MOSI! User_Setup.h not updated in library!");
    Serial.println("Copy User_Setup.h to TFT_eSPI library folder and re-upload.");
    return;
  }

  Serial.println("\n[1] tft.init()...");
  Serial.flush();
  tft.init();
  Serial.println("    OK!");

  tft.setRotation(0);
  tft.setSwapBytes(true);

  Serial.println("[2] Fill RED...");
  tft.fillScreen(TFT_RED);
  delay(1000);

  Serial.println("[3] Fill GREEN...");
  tft.fillScreen(TFT_GREEN);
  delay(1000);

  Serial.println("[4] Fill BLUE...");
  tft.fillScreen(TFT_BLUE);
  delay(1000);

  Serial.println("[5] Sprite test...");
  fb.setColorDepth(16);
  void *ptr = fb.createSprite(240, 240);
  if (!ptr) {
    Serial.printf("    [FAIL] createSprite failed! Heap: %d\n", ESP.getFreeHeap());
    return;
  }

  fb.setSwapBytes(true);
  fb.fillSprite(TFT_BLACK);
  fb.setTextColor(TFT_CYAN);
  fb.setTextSize(2);
  fb.setCursor(30, 40);
  fb.println("TamaFi v2");
  fb.setTextSize(1);
  fb.setTextColor(TFT_WHITE);
  fb.setCursor(30, 80);
  fb.println("TFT_eSPI OK!");
  fb.setCursor(30, 100);
  fb.println("Sprite OK!");
  fb.setCursor(30, 120);
  fb.printf("Heap: %d bytes", ESP.getFreeHeap());
  fb.drawRect(5, 5, 230, 230, TFT_YELLOW);
  fb.pushSprite(0, 0);

  Serial.println("    OK! Screen should show 'TamaFi v2'");
  Serial.println("\n=== ALL PASSED ===");
}

void loop() {}
