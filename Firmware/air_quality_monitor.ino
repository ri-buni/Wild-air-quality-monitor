/*
 * Air Quality Monitor - ESP32-S3 DevKitC
 * ---------------------------------------
 * SCD41 -> CO2
 * SHT41 -> Temperature / Humidity (also feeds SGP40 compensation)
 * SGP40 -> VOC Index (via Sensirion Gas Index Algorithm)
 * Display: SSD1306/SSD1309 128x64 OLED over I2C
 * Power: TP4056 + Li-ion (charging handled by TP4056 hardware)
 *
 * Libraries (Arduino Library Manager):
 *   - Sensirion I2C SCD4x
 *   - Sensirion I2C SHT4x
 *   - Sensirion I2C SGP40
 *   - Sensirion Gas Index Algorithm
 *   - U8g2
 *
 * Wiring: all four I2C devices share one bus.
 *   SDA = GPIO8, SCL = GPIO9 (ESP32-S3 DevKitC defaults)
 *   Addresses: SCD41 0x62 | SHT41 0x44 | SGP40 0x59 | OLED 0x3C
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SensirionI2cScd4x.h>
#include <SensirionI2cSht4x.h>
#include <SensirionI2CSgp40.h>
#include <VOCGasIndexAlgorithm.h>

// ---------- I2C pins (ESP32-S3 DevKitC defaults) ----------
#define I2C_SDA 8
#define I2C_SCL 9

// ---------- Sampling ----------
const uint32_t SAMPLE_INTERVAL_MS = 1000;   // SGP40 must be fed at a CONSTANT 1 Hz
const float    DEFAULT_TEMP_C     = 25.0f;
const float    DEFAULT_RH         = 50.0f;

// ---------- Objects ----------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// For a true SSD1309 panel, swap the line above for:
// U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

SensirionI2cScd4x scd41;
SensirionI2cSht4x sht41;
SensirionI2CSgp40 sgp40;
VOCGasIndexAlgorithm vocAlgorithm;   // default 1 Hz sampling interval

// ---------- Shared state ----------
float    g_temp = DEFAULT_TEMP_C;
float    g_rh   = DEFAULT_RH;
uint16_t g_co2  = 0;
int32_t  g_voc  = 0;
bool     g_scdReady = false;

uint32_t lastSample = 0;

// =====================================================================
//  Helpers
// =====================================================================
static void haltWithError(const char* msg) {
  Serial.println(msg);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 12, "Init error:");
  u8g2.drawStr(0, 28, msg);
  u8g2.sendBuffer();
  while (true) delay(1000);
}

// Status levels: 0 = noice, 1 = okei, 2 = nah, 3 = Xtreme
uint8_t co2Level(uint16_t c) {
  if (c == 0)    return 0;
  if (c < 800)   return 0;
  if (c < 1200)  return 1;
  if (c < 1600)  return 2;
  return 3;
}
uint8_t vocLevel(int32_t v) {
  if (v < 100) return 0;
  if (v < 200) return 1;
  if (v < 300) return 2;
  return 3;
}
const char* levelWord(uint8_t lvl) {
  switch (lvl) { case 0: return "noice";  case 1: return "okei";
                 case 2: return "nah";    default: return "Xtreme"; }
}

// Map CO2 ppm -> bar fill width (0..maxw), range 400..1600 ppm
uint8_t co2BarWidth(uint16_t c, uint8_t maxw) {
  if (c < 400) c = 400;
  long w = (long)(c - 400) * maxw / 1200;
  if (w > maxw) w = maxw;
  return (uint8_t)w;
}

// =====================================================================
//  Sensor reads
// =====================================================================
void readSht41() {
  float t, rh;
  uint16_t err = sht41.measureHighPrecision(t, rh);
  if (!err) {
    g_temp = t;
    g_rh   = rh;
  }
}

void readScd41() {
  bool dataReady = false;
  if (scd41.getDataReadyStatus(dataReady) == 0 && dataReady) {
    uint16_t co2;
    float t, rh;   // SCD41 also reports T/RH; we keep only CO2 per the sensor split
    if (scd41.readMeasurement(co2, t, rh) == 0 && co2 != 0) {
      g_co2 = co2;
      g_scdReady = true;
    }
  }
}

void readSgp40() {
  // Convert SHT41 T/RH into SGP40 compensation ticks
  uint16_t rhTicks   = (uint16_t)((g_rh * 65535) / 100.0f + 0.5f);
  uint16_t tempTicks = (uint16_t)(((g_temp + 45) * 65535) / 175.0f + 0.5f);

  uint16_t sraw = 0;
  uint16_t err = sgp40.measureRawSignal(rhTicks, tempTicks, sraw);
  if (!err) {
    g_voc = vocAlgorithm.process((int32_t)sraw);
  }
}

// =====================================================================
//  Display
// =====================================================================
void draw() {
  char buf[24];
  u8g2.clearBuffer();

  // ---- Header bar (white) with title + status pill ----
  uint8_t overall = max(co2Level(g_co2), vocLevel(g_voc));
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 14);

  u8g2.setFont(u8g2_font_logisoso16_tr);  // header in the logisoso family
  u8g2.setDrawColor(0);                    // black text on white bar
  u8g2.drawStr(3, 13, "ri's grade");

  // status pill in 4x6 so even "Xtreme" never collides with the header
  u8g2.setFont(u8g2_font_4x6_tr);
  const char* word = levelWord(overall);
  uint8_t pw = u8g2.getStrWidth(word) + 4;
  uint8_t px = 127 - pw;
  u8g2.setDrawColor(0);                     // black pill cut into the white bar
  u8g2.drawRBox(px, 3, pw, 8, 1);
  u8g2.setDrawColor(1);                     // white text in the pill
  u8g2.drawStr(px + 2, 9, word);

  // ---- CO2 hero number ----
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_logisoso24_tn);
  snprintf(buf, sizeof(buf), "%u", g_scdReady ? g_co2 : 0);
  if (!g_scdReady) strcpy(buf, "0");
  u8g2.drawStr(3, 43, buf);
  uint8_t numW = u8g2.getStrWidth(buf);

  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(numW + 8, 25, "CO2");
  u8g2.drawStr(numW + 8, 39, "ppm");

  // ---- CO2 level bar with threshold ticks (800 / 1200 / 1600 ppm) ----
  u8g2.drawVLine(33, 44, 2);
  u8g2.drawVLine(64, 44, 2);
  u8g2.drawVLine(95, 44, 2);
  u8g2.drawFrame(0, 46, 128, 6);
  uint8_t bw = co2BarWidth(g_scdReady ? g_co2 : 0, 124);
  if (bw > 0) u8g2.drawBox(2, 48, bw, 2);

  // ---- Footer: temp | humidity | VOC ----
  u8g2.drawVLine(43, 54, 9);
  u8g2.drawVLine(85, 54, 9);

  // thermometer icon
  u8g2.drawVLine(4, 55, 5);
  u8g2.drawDisc(4, 61, 1);
  u8g2.setFont(u8g2_font_5x8_tr);
  snprintf(buf, sizeof(buf), "%.1f", g_temp);
  u8g2.drawStr(9, 62, buf);
  u8g2.drawStr(9 + u8g2.getStrWidth(buf), 62, "\xb0");  // degree

  // droplet icon
  u8g2.drawDisc(48, 60, 2);
  u8g2.drawTriangle(48, 55, 46, 59, 50, 59);
  snprintf(buf, sizeof(buf), "%.0f%%", g_rh);
  u8g2.drawStr(53, 62, buf);

  // VOC
  snprintf(buf, sizeof(buf), "VOC %ld", (long)g_voc);
  u8g2.drawStr(88, 62, buf);

  u8g2.sendBuffer();
}

// =====================================================================
//  Setup / Loop
// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);   // 100 kHz is safe for all four devices

  // Display splash
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, "ri's grade");
  u8g2.drawStr(0, 30, "Warming up...");
  u8g2.sendBuffer();

  char errMsg[64];
  uint16_t err;

  // --- SHT41 ---
  sht41.begin(Wire, SHT41_I2C_ADDR_44);
  sht41.softReset();
  delay(20);

  // --- SCD41 ---
  scd41.begin(Wire, SCD41_I2C_ADDR_62);
  scd41.wakeUp();
  scd41.stopPeriodicMeasurement();   // ensure a clean state
  delay(500);
  scd41.reinit();
  delay(20);
  err = scd41.startPeriodicMeasurement();   // new reading ~every 5 s
  if (err) {
    errorToString(err, errMsg, sizeof(errMsg));
    haltWithError(errMsg);
  }

  // --- SGP40 ---
  sgp40.begin(Wire);
  uint16_t testRaw = 0;
  err = sgp40.measureRawSignal(0x8000, 0x6666, testRaw);  // ping with defaults
  if (err) {
    errorToString(err, errMsg, sizeof(errMsg));
    Serial.print("SGP40 warning: ");
    Serial.println(errMsg);
  }

  lastSample = millis();
}

void loop() {
  uint32_t now = millis();
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample += SAMPLE_INTERVAL_MS;

    readSht41();   // 1) temperature + humidity
    readScd41();   // 2) CO2 (updates ~every 5 s)
    readSgp40();   // 3) VOC, compensated by SHT41 T/RH

    draw();

    Serial.printf("CO2=%u ppm  VOC=%ld  T=%.2f C  RH=%.1f %%\n",
                  g_co2, (long)g_voc, g_temp, g_rh);
  }
}
