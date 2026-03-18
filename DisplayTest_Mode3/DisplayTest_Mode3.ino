#include <Arduino.h>
#include <SPI.h>

static const int TFT_SCLK = 18;
static const int TFT_MOSI = 17;
static const int TFT_CS   = 16;
static const int TFT_DC   = 15;
static const int TFT_RES  = 4;

SPIClass spi(FSPI);
SPISettings tftSPI(10000000, MSBFIRST, SPI_MODE0);  // MODE3 → MODE0

void writeCommand(uint8_t cmd) {
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CS, LOW);
  spi.beginTransaction(tftSPI);
  spi.transfer(cmd);
  spi.endTransaction();
  digitalWrite(TFT_CS, HIGH);
}

void writeData(const uint8_t *data, size_t len) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  spi.beginTransaction(tftSPI);
  for (size_t i = 0; i < len; ++i) spi.transfer(data[i]);
  spi.endTransaction();
  digitalWrite(TFT_CS, HIGH);
}

void writeDataByte(uint8_t data) {
  writeData(&data, 1);
}

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  uint8_t data[4];

  writeCommand(0x2A);
  data[0] = x0 >> 8;
  data[1] = x0 & 0xFF;
  data[2] = x1 >> 8;
  data[3] = x1 & 0xFF;
  writeData(data, 4);

  writeCommand(0x2B);
  data[0] = y0 >> 8;
  data[1] = y0 & 0xFF;
  data[2] = y1 >> 8;
  data[3] = y1 & 0xFF;
  writeData(data, 4);

  writeCommand(0x2C);
}

void fillColor(uint16_t color) {
  setAddrWindow(0, 0, 239, 239);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  spi.beginTransaction(tftSPI);
  uint8_t hi = color >> 8;
  uint8_t lo = color & 0xFF;
  for (uint32_t i = 0; i < 240UL * 240UL; ++i) {
    spi.transfer(hi);
    spi.transfer(lo);
  }
  spi.endTransaction();
  digitalWrite(TFT_CS, HIGH);
}

void resetTFT() {
  digitalWrite(TFT_RES, LOW);
  delay(100);
  digitalWrite(TFT_RES, HIGH);
  delay(100);
}

void initTFT() {
  resetTFT();

  writeCommand(0x11);
  delay(120);

  writeCommand(0x36);
  writeDataByte(0x00);

  writeCommand(0x3A);
  writeDataByte(0x05);

  writeCommand(0xB2);
  const uint8_t b2[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
  writeData(b2, sizeof(b2));

  writeCommand(0xB7);
  writeDataByte(0x35);

  writeCommand(0xBB);
  writeDataByte(0x32);

  writeCommand(0xC2);
  writeDataByte(0x01);

  writeCommand(0xC3);
  writeDataByte(0x15);

  writeCommand(0xC4);
  writeDataByte(0x20);

  writeCommand(0xC6);
  writeDataByte(0x0F);

  writeCommand(0xD0);
  const uint8_t d0[] = {0xA4, 0xA1};
  writeData(d0, sizeof(d0));

  writeCommand(0xE0);
  const uint8_t e0[] = {0xD0, 0x08, 0x0E, 0x09, 0x09, 0x05, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34};
  writeData(e0, sizeof(e0));

  writeCommand(0xE1);
  const uint8_t e1[] = {0xD0, 0x08, 0x0E, 0x09, 0x09, 0x15, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34};
  writeData(e1, sizeof(e1));

  writeCommand(0x21);
  writeCommand(0x29);
  delay(20);
}

void setup() {
  // 背光 — 根据实际接线设置引脚号
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, HIGH);

  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RES, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_RES, HIGH);

  spi.begin(TFT_SCLK, -1, TFT_MOSI, -1);  // 关键修改：CS 传 -1
  initTFT();

  fillColor(0xF800);
  delay(1000);
  fillColor(0x07E0);
  delay(1000);
  fillColor(0x001F);
  delay(1000);
  fillColor(0x0000);
}

void loop() {
}
