/*
 * TamaFi Wiring Diagnostic v3
 * 
 * Since raw SPI also shows nothing, this tests multiple
 * wiring combinations to find the correct one.
 * 
 * Open Serial Monitor at 115200 baud.
 * Watch the screen carefully during each test.
 */

#include <Arduino.h>
#include <SPI.h>

SPIClass rawSpi(FSPI);

// ----- Raw ST7789 driver -----
static int g_cs, g_dc, g_rst;

void rawCmd(uint8_t cmd) {
  digitalWrite(g_dc, LOW);
  digitalWrite(g_cs, LOW);
  rawSpi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  rawSpi.transfer(cmd);
  rawSpi.endTransaction();
  digitalWrite(g_cs, HIGH);
}

void rawDataByte(uint8_t v) {
  digitalWrite(g_dc, HIGH);
  digitalWrite(g_cs, LOW);
  rawSpi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  rawSpi.transfer(v);
  rawSpi.endTransaction();
  digitalWrite(g_cs, HIGH);
}

void rawData(const uint8_t *d, size_t len) {
  digitalWrite(g_dc, HIGH);
  digitalWrite(g_cs, LOW);
  rawSpi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  for (size_t i = 0; i < len; i++) rawSpi.transfer(d[i]);
  rawSpi.endTransaction();
  digitalWrite(g_cs, HIGH);
}

void rawFillColor(uint16_t color, int offX, int offY) {
  uint8_t buf[4];
  uint16_t x0 = offX, y0 = offY;
  uint16_t x1 = offX + 239, y1 = offY + 239;

  rawCmd(0x2A);
  buf[0]=x0>>8; buf[1]=x0; buf[2]=x1>>8; buf[3]=x1;
  rawData(buf, 4);

  rawCmd(0x2B);
  buf[0]=y0>>8; buf[1]=y0; buf[2]=y1>>8; buf[3]=y1;
  rawData(buf, 4);

  rawCmd(0x2C);
  digitalWrite(g_dc, HIGH);
  digitalWrite(g_cs, LOW);
  rawSpi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  uint8_t hi = color >> 8, lo = color & 0xFF;
  for (uint32_t i = 0; i < 240UL * 240UL; i++) {
    rawSpi.transfer(hi);
    rawSpi.transfer(lo);
  }
  rawSpi.endTransaction();
  digitalWrite(g_cs, HIGH);
}

void initST7789(uint8_t madctl) {
  digitalWrite(g_rst, LOW);  delay(100);
  digitalWrite(g_rst, HIGH); delay(120);

  rawCmd(0x01); delay(150);  // Software reset
  rawCmd(0x11); delay(120);  // Sleep out

  rawCmd(0x36); rawDataByte(madctl);   // MADCTL
  rawCmd(0x3A); rawDataByte(0x05);     // 16-bit color

  rawCmd(0xB2);
  const uint8_t b2[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
  rawData(b2, sizeof(b2));

  rawCmd(0xB7); rawDataByte(0x35);
  rawCmd(0xBB); rawDataByte(0x32);
  rawCmd(0xC0); rawDataByte(0x2C);
  rawCmd(0xC2); rawDataByte(0x01);
  rawCmd(0xC3); rawDataByte(0x15);
  rawCmd(0xC4); rawDataByte(0x20);
  rawCmd(0xC6); rawDataByte(0x0F);
  rawCmd(0xD0);
  const uint8_t d0[] = {0xA4, 0xA1};
  rawData(d0, sizeof(d0));

  rawCmd(0x21);         // Inversion ON
  rawCmd(0x29);         // Display ON
  delay(50);
}

// ===== Run a single test with given pin config =====
void runTest(const char* label, int sclk, int mosi, int cs, int dc, int rst) {
  Serial.printf("\n--- %s ---\n", label);
  Serial.printf("  SCLK=%d  MOSI=%d  CS=%d  DC=%d  RST=%d\n", sclk, mosi, cs, dc, rst);

  g_cs = cs; g_dc = dc; g_rst = rst;

  pinMode(cs, OUTPUT);  digitalWrite(cs, HIGH);
  pinMode(dc, OUTPUT);  digitalWrite(dc, HIGH);
  pinMode(rst, OUTPUT); digitalWrite(rst, HIGH);

  rawSpi.end();
  rawSpi.begin(sclk, -1, mosi, -1);

  // Test offset 0,0
  initST7789(0x00);
  Serial.println("  RED (offset 0,0)...");
  rawFillColor(0xF800, 0, 0);
  delay(2000);

  // Test offset 0,80 (common for 240x240 on 240x320 controller)
  initST7789(0x00);
  Serial.println("  GREEN (offset 0,80)...");
  rawFillColor(0x07E0, 0, 80);
  delay(2000);

  // Test with inverted color order (BGR instead of RGB)
  initST7789(0x08);
  Serial.println("  BLUE (BGR mode, offset 0,0)...");
  rawFillColor(0x001F, 0, 0);
  delay(2000);

  Serial.printf("  >>> Did you see ANY color during '%s'?\n", label);
  rawSpi.end();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("========================================");
  Serial.println("   TamaFi Wiring Diagnostic v3");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Watch the screen carefully during each test.");
  Serial.println("Report which test (if any) showed color.");
  Serial.println();

  // ===== Test 1: Original pin config =====
  // SCLK=18, MOSI=17, CS=16, DC=15, RST=4
  runTest("Test 1: Original (SCLK=18, MOSI=17)",
          18, 17, 16, 15, 4);

  // ===== Test 2: MOSI/SCLK swapped =====
  // Maybe user swapped the data and clock wires
  runTest("Test 2: MOSI/SCLK swapped (SCLK=17, MOSI=18)",
          17, 18, 16, 15, 4);

  // ===== Test 3: CS/DC swapped =====
  // Maybe user swapped CS and DC wires
  runTest("Test 3: CS/DC swapped (CS=15, DC=16)",
          18, 17, 15, 16, 4);

  // ===== Test 4: All signal wires shifted =====
  // Maybe pins are off by one position on breadboard
  runTest("Test 4: Shifted (SCLK=17, MOSI=16, CS=15, DC=4, RST=18)",
          17, 16, 15, 4, 18);

  Serial.println("\n\n========================================");
  Serial.println("   All tests done.");
  Serial.println("========================================");
  Serial.println();
  Serial.println("If NO test showed any color:");
  Serial.println("  1. Check: is your screen REALLY ST7789?");
  Serial.println("     (look at the text printed on the screen PCB)");
  Serial.println("  2. Check: are VCC and GND definitely correct?");
  Serial.println("  3. Try: disconnect ALL wires, reconnect one by one");
  Serial.println("  4. Check: are any wires loose or not making contact?");
  Serial.println();
  Serial.println("If ONE test showed color, tell me which one.");
  Serial.println("That tells me exactly which wires need to be swapped.");
}

void loop() {}
