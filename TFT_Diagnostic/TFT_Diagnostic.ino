/*
 * TFT_eSPI Diagnostic Sketch for TamaFi
 * 
 * Upload this, then open Serial Monitor (115200 baud).
 * It will print all TFT_eSPI configuration info and test
 * the display step by step.
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite fb(&tft);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("========================================");
  Serial.println("   TamaFi TFT Diagnostic Tool");
  Serial.println("========================================");

  // ---- Step 1: Memory info ----
  Serial.println("\n[1] Memory Info");
  Serial.printf("  Total heap:  %d bytes\n", ESP.getHeapSize());
  Serial.printf("  Free heap:   %d bytes\n", ESP.getFreeHeap());
  Serial.printf("  PSRAM size:  %d bytes\n", ESP.getPsramSize());
  Serial.printf("  Free PSRAM:  %d bytes\n", ESP.getFreePsram());

  // ---- Step 2: TFT_eSPI setup info ----
  Serial.println("\n[2] TFT_eSPI Configuration");
  setup_t tftSetup;
  tft.getSetup(tftSetup);

  Serial.printf("  TFT_eSPI version: %s\n", tftSetup.version.c_str());
  Serial.printf("  Driver:  0x%04X\n", tftSetup.setup_id);
  Serial.printf("  SPI port: %d\n", tftSetup.port);
  Serial.printf("  MOSI pin: %d\n", tftSetup.pin_tft_mosi);
  Serial.printf("  MISO pin: %d\n", tftSetup.pin_tft_miso);
  Serial.printf("  SCLK pin: %d\n", tftSetup.pin_tft_clk);
  Serial.printf("  CS pin:   %d\n", tftSetup.pin_tft_cs);
  Serial.printf("  DC pin:   %d\n", tftSetup.pin_tft_dc);
  Serial.printf("  RST pin:  %d\n", tftSetup.pin_tft_rst);

  Serial.println("\n  >>> Expected pins: MOSI=17, SCLK=18, CS=16, DC=15, RST=4");
  Serial.println("  >>> If pins above DON'T match, User_Setup.h was NOT loaded correctly!");

  bool pinsOK = (tftSetup.pin_tft_mosi == 17 &&
                 tftSetup.pin_tft_clk  == 18 &&
                 tftSetup.pin_tft_cs   == 16 &&
                 tftSetup.pin_tft_dc   == 15 &&
                 tftSetup.pin_tft_rst  == 4);

  if (pinsOK) {
    Serial.println("  [OK] Pins match expected configuration.");
  } else {
    Serial.println("  [FAIL] Pins DO NOT match! User_Setup.h is NOT being used.");
    Serial.println("  --> Copy User_Setup.h to your TFT_eSPI library folder.");
    Serial.println("  --> Also check User_Setup_Select.h in the TFT_eSPI library.");
  }

  // ---- Step 3: Init TFT ----
  Serial.println("\n[3] Initializing TFT...");
  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  Serial.println("  tft.init() done.");

  // ---- Step 4: Direct draw test (no sprite) ----
  Serial.println("\n[4] Direct draw test (no sprite buffer)");

  Serial.println("  Filling RED...");
  tft.fillScreen(TFT_RED);
  delay(1000);

  Serial.println("  Filling GREEN...");
  tft.fillScreen(TFT_GREEN);
  delay(1000);

  Serial.println("  Filling BLUE...");
  tft.fillScreen(TFT_BLUE);
  delay(1000);

  Serial.println("  Drawing text on BLACK...");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 10);
  tft.println("Direct draw OK!");
  tft.setCursor(10, 30);
  tft.printf("Free heap: %d", ESP.getFreeHeap());
  delay(2000);

  Serial.println("  >>> Did you see RED, GREEN, BLUE, then text?");
  Serial.println("  >>> If YES: TFT_eSPI works. Issue is in sprite allocation.");
  Serial.println("  >>> If NO: TFT_eSPI config or SPI bus is wrong.");

  // ---- Step 5: Sprite test ----
  Serial.println("\n[5] Sprite allocation test");
  Serial.printf("  Free heap before sprite: %d bytes\n", ESP.getFreeHeap());

  fb.setColorDepth(16);
  void* ptr = fb.createSprite(240, 240);

  if (ptr == NULL) {
    Serial.println("  [FAIL] createSprite(240,240) FAILED - not enough RAM!");
    Serial.println("  >>> This is why the main program shows a black screen.");
    Serial.println("  >>> The framebuffer could not be allocated.");

    Serial.println("\n  Trying smaller sprite (120x120)...");
    ptr = fb.createSprite(120, 120);
    if (ptr) {
      Serial.println("  [OK] 120x120 sprite works. Full 240x240 needs more RAM.");
    } else {
      Serial.println("  [FAIL] Even 120x120 failed. Severe memory issue.");
    }
  } else {
    Serial.println("  [OK] createSprite(240,240) succeeded!");
    Serial.printf("  Free heap after sprite: %d bytes\n", ESP.getFreeHeap());

    fb.setSwapBytes(true);
    fb.fillSprite(TFT_BLACK);
    fb.setTextColor(TFT_CYAN);
    fb.setCursor(10, 10);
    fb.println("Sprite draw OK!");
    fb.setCursor(10, 30);
    fb.println("TFT_eSPI is working.");
    fb.setCursor(10, 50);
    fb.printf("Heap left: %d", ESP.getFreeHeap());

    fb.drawRect(5, 5, 230, 230, TFT_YELLOW);

    fb.pushSprite(0, 0);

    Serial.println("  >>> Did you see 'Sprite draw OK!' with yellow border?");
    Serial.println("  >>> If YES: Everything works. Main program should also work.");
  }

  Serial.println("\n========================================");
  Serial.println("   Diagnostic complete.");
  Serial.println("   Check serial output for [FAIL] items.");
  Serial.println("========================================");
}

void loop() {
}
