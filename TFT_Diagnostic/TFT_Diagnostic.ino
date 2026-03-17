/*
 * TamaFi TFT Diagnostic v2
 * 
 * Part A: Raw SPI test (bypasses TFT_eSPI entirely)
 * Part B: TFT_eSPI test with detailed error info
 * 
 * Open Serial Monitor at 115200 baud.
 */

#include <Arduino.h>
#include <SPI.h>

// ===== Hardware pins (matching User_Setup.h & DisplayTest_Mode3) =====
#define PIN_SCLK 18
#define PIN_MOSI 17
#define PIN_CS   16
#define PIN_DC   15
#define PIN_RST  4

SPIClass rawSpi(FSPI);
SPISettings rawSpiSettings(10000000, MSBFIRST, SPI_MODE0);

// ----- Raw ST7789 helpers -----
void rawCmd(uint8_t cmd) {
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_CS, LOW);
  rawSpi.beginTransaction(rawSpiSettings);
  rawSpi.transfer(cmd);
  rawSpi.endTransaction();
  digitalWrite(PIN_CS, HIGH);
}

void rawData(const uint8_t *d, size_t len) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  rawSpi.beginTransaction(rawSpiSettings);
  for (size_t i = 0; i < len; i++) rawSpi.transfer(d[i]);
  rawSpi.endTransaction();
  digitalWrite(PIN_CS, HIGH);
}

void rawDataByte(uint8_t v) { rawData(&v, 1); }

void rawSetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  uint8_t buf[4];
  rawCmd(0x2A);
  buf[0]=x0>>8; buf[1]=x0; buf[2]=x1>>8; buf[3]=x1;
  rawData(buf,4);
  rawCmd(0x2B);
  buf[0]=y0>>8; buf[1]=y0; buf[2]=y1>>8; buf[3]=y1;
  rawData(buf,4);
  rawCmd(0x2C);
}

void rawFill(uint16_t color) {
  rawSetWindow(0, 0, 239, 239);
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  rawSpi.beginTransaction(rawSpiSettings);
  uint8_t hi = color >> 8, lo = color & 0xFF;
  for (uint32_t i = 0; i < 240UL*240UL; i++) {
    rawSpi.transfer(hi);
    rawSpi.transfer(lo);
  }
  rawSpi.endTransaction();
  digitalWrite(PIN_CS, HIGH);
}

void rawReset() {
  digitalWrite(PIN_RST, LOW);  delay(100);
  digitalWrite(PIN_RST, HIGH); delay(100);
}

void rawInitST7789() {
  rawReset();
  rawCmd(0x11); delay(120);            // Sleep out
  rawCmd(0x36); rawDataByte(0x00);     // MADCTL
  rawCmd(0x3A); rawDataByte(0x05);     // 16-bit color
  rawCmd(0xB2);                        // Porch control
  const uint8_t b2[] = {0x0C,0x0C,0x00,0x33,0x33};
  rawData(b2, sizeof(b2));
  rawCmd(0xB7); rawDataByte(0x35);     // Gate control
  rawCmd(0xBB); rawDataByte(0x32);     // VCOM
  rawCmd(0xC2); rawDataByte(0x01);     // VDV/VRH enable
  rawCmd(0xC3); rawDataByte(0x15);     // VRH
  rawCmd(0xC4); rawDataByte(0x20);     // VDV
  rawCmd(0xC6); rawDataByte(0x0F);     // Frame rate
  rawCmd(0xD0);                        // Power control
  const uint8_t d0[] = {0xA4, 0xA1};
  rawData(d0, sizeof(d0));
  rawCmd(0x21);                        // Inversion ON
  rawCmd(0x29); delay(20);             // Display ON
}

// ===== PART A: Raw SPI test =====
bool testRawSPI() {
  Serial.println("\n============ PART A: Raw SPI Test ============");
  Serial.println("  (Bypasses TFT_eSPI completely)\n");

  pinMode(PIN_CS,  OUTPUT); digitalWrite(PIN_CS,  HIGH);
  pinMode(PIN_DC,  OUTPUT); digitalWrite(PIN_DC,  HIGH);
  pinMode(PIN_RST, OUTPUT); digitalWrite(PIN_RST, HIGH);

  Serial.printf("  Pins: SCLK=%d, MOSI=%d, CS=%d, DC=%d, RST=%d\n",
                PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST);

  Serial.println("  Starting SPI (FSPI)...");
  rawSpi.begin(PIN_SCLK, -1, PIN_MOSI, -1);
  Serial.println("  SPI started OK.");

  Serial.println("  Initializing ST7789...");
  rawInitST7789();
  Serial.println("  ST7789 init done.");

  Serial.println("\n  Filling RED...");
  rawFill(0xF800);
  delay(1500);

  Serial.println("  Filling GREEN...");
  rawFill(0x07E0);
  delay(1500);

  Serial.println("  Filling BLUE...");
  rawFill(0x001F);
  delay(1500);

  Serial.println("\n  >>> Did you see RED, GREEN, BLUE on the screen?");
  Serial.println("  >>> If YES: hardware + wiring is 100% correct.");
  Serial.println("  >>> If NO:  check wiring carefully.");

  rawSpi.end();
  return true;
}

// ===== PART B: TFT_eSPI test =====
#include <TFT_eSPI.h>

void testTftEspi() {
  Serial.println("\n\n============ PART B: TFT_eSPI Test ============\n");

  TFT_eSPI tft = TFT_eSPI();

  Serial.println("[B1] TFT_eSPI compile-time defines:");
  #ifdef TFT_MISO
    Serial.printf("  TFT_MISO = %d\n", TFT_MISO);
  #else
    Serial.println("  TFT_MISO = NOT DEFINED (will use default -> may conflict!)");
  #endif
  Serial.printf("  TFT_MOSI = %d\n", TFT_MOSI);
  Serial.printf("  TFT_SCLK = %d\n", TFT_SCLK);
  Serial.printf("  TFT_CS   = %d\n", TFT_CS);
  Serial.printf("  TFT_DC   = %d\n", TFT_DC);
  Serial.printf("  TFT_RST  = %d\n", TFT_RST);

  #ifdef USE_FSPI_PORT
    Serial.println("  USE_FSPI_PORT = DEFINED");
  #else
    Serial.println("  USE_FSPI_PORT = not defined");
  #endif
  #ifdef USE_HSPI_PORT
    Serial.println("  USE_HSPI_PORT = DEFINED");
  #else
    Serial.println("  USE_HSPI_PORT = not defined");
  #endif

  Serial.println("\n[B2] Calling tft.init()...");
  Serial.println("  (If it hangs here, TFT_eSPI SPI config is broken)");
  Serial.flush();

  unsigned long t0 = millis();
  tft.init();
  unsigned long t1 = millis();

  Serial.printf("  tft.init() completed in %lu ms\n", t1 - t0);

  tft.setRotation(0);
  tft.setSwapBytes(true);

  Serial.println("\n[B3] Direct fill test...");
  tft.fillScreen(TFT_RED);
  delay(1000);
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  tft.fillScreen(TFT_BLUE);
  delay(1000);

  Serial.println("  >>> Did you see RED, GREEN, BLUE?");

  Serial.println("\n[B4] Sprite allocation test...");
  TFT_eSprite fb(&tft);
  fb.setColorDepth(16);
  void *ptr = fb.createSprite(240, 240);
  if (!ptr) {
    Serial.println("  [FAIL] createSprite(240,240) failed!");
    Serial.printf("  Free heap: %d bytes\n", ESP.getFreeHeap());
  } else {
    Serial.println("  [OK] createSprite(240,240) succeeded.");
    fb.fillSprite(TFT_BLACK);
    fb.setTextColor(TFT_WHITE);
    fb.setCursor(20, 20);
    fb.setTextSize(2);
    fb.println("TamaFi OK!");
    fb.setTextSize(1);
    fb.setCursor(20, 60);
    fb.println("TFT_eSPI + Sprite working.");
    fb.setCursor(20, 80);
    fb.printf("Heap: %d bytes", ESP.getFreeHeap());
    fb.pushSprite(0, 0);
    Serial.println("  >>> Screen should show 'TamaFi OK!'");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("========================================");
  Serial.println("   TamaFi TFT Diagnostic v2");
  Serial.println("========================================");

  Serial.printf("\nChip:       %s\n", ESP.getChipModel());
  Serial.printf("Free heap:  %d bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM:      %d bytes\n", ESP.getPsramSize());

  // Part A: raw SPI (always works if wiring is correct)
  testRawSPI();

  delay(2000);

  // Part B: TFT_eSPI
  testTftEspi();

  Serial.println("\n========================================");
  Serial.println("   Diagnostic complete.");
  Serial.println("========================================");
}

void loop() {}
