#include <Arduino.h>

static const uint8_t BUZZER_PIN = 2;

void beep(int freq, int ms) {
  ledcWriteTone(BUZZER_PIN, freq);
  delay(ms);
  ledcWriteTone(BUZZER_PIN, 0);
  delay(200);
}

void setup() {
  ledcAttach(BUZZER_PIN, 2000, 8);
  ledcWriteTone(BUZZER_PIN, 0);
}

void loop() {
  beep(800, 300);
  beep(1200, 300);
  beep(1600, 300);
  delay(1000);
}
