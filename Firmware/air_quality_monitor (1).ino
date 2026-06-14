/*
 * Air Quality Monitor - ESP32-S3 DevKitC
 * ---------------------------------------
 * SCD41   -> CO2
 * SHT41   -> Temperature / Humidity (also feeds SGP40 compensation)
 * SGP40   -> VOC Index (via Sensirion Gas Index Algorithm)
 * PMS5003 -> PM1.0 / PM2.5 / PM10 (UART, NOT I2C)
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
 * Wiring (I2C): SCD41 / SHT41 / SGP40 / OLED share one bus.
 *   SDA = GPIO8, SCL = GPIO9 (ESP32-S3 DevKitC defaults)
 *   Addresses: SCD41 0x62 | SHT41 0x44 | SGP40 0x59 | OLED 0x3C
 *
 * Wiring (PMS5003 UART, 5V VCC, 3.3V logic - safe for ESP32-S3 RX):
 *   PMS5003 TX (pin 5) -> ESP32 GPIO17 (PMS_RX)
 *   PMS5003 RX (pin 4) -> ESP32 GPIO18 (PMS_TX, optional - for sleep/wake cmds)
 *   PMS5003 VCC -> 5V    GND -> GND
 *   SET / RESET left floating (internal pull-ups keep the sensor running)
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

// ---------- PMS5003 UART (Serial1) ----------
#define PMS_RX  17   // <- PMS5003 TX
#define PMS_TX  18   // -> PMS5003 RX (optional)
#define PMS_BAUD 9600
#define PMS_FRAME_LEN 32
HardwareSerial PmsSerial(1);

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
uint16_t g_pm1  = 0;   // PM1.0  (ug/m3, atmospheric)
uint16_t g_pm25 = 0;   // PM2.5  (ug/m3, atmospheric)
uint16_t g_pm10 = 0;   // PM10   (ug/m3, atmospheric)
bool     g_scdReady = false;
bool     g_pmReady  = false;

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
// PM2.5 ug/m3, loosely following US EPA 24h AQI breakpoints
uint8_t pm25Level(uint16_t p) {
  if (p <= 12)  return 0;   // good
  if (p <= 35)  return 1;   // moderate
  if (p <= 55)  return 2;   // unhealthy for sensitive
  return 3;                 // unhealthy+
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

// ---------------------------------------------------------------------
// Bunny mascot (earless). Drawn from primitives in a ~22x26 box whose
// top-left is (x, y). The face is symmetric about its vertical center.
// Expression changes with the air-quality level:
//   0 noice  -> ^ ^ happy eyes, smile
//   1 okei   -> o o dot eyes, flat mouth
//   2 nah    -> closed eyes (— —), flat mouth
//   3 Xtreme -> x x eyes, frown
// All strokes use the current draw color (set to 1/white by caller).
void drawBunny(uint8_t x, uint8_t y, uint8_t lvl) {
  // Head: rounded box, vertically roomy since there are no ears
  uint8_t hx = x + 2, hy = y + 4, hw = 18, hh = 18;
  u8g2.drawRFrame(hx, hy, hw, hh, 5);

  // Symmetry axis of the head
  uint8_t cx = hx + hw / 2;        // center column
  uint8_t ey = hy + 7;             // eye row
  uint8_t eo = 4;                  // eye offset from center (mirrored)
  uint8_t lx = cx - eo;            // left eye center
  uint8_t rx = cx + eo;            // right eye center

  if (lvl == 0) {
    // ^ ^ happy eyes (mirrored chevrons)
    u8g2.drawLine(lx - 2, ey, lx, ey - 2);
    u8g2.drawLine(lx, ey - 2, lx + 2, ey);
    u8g2.drawLine(rx - 2, ey, rx, ey - 2);
    u8g2.drawLine(rx, ey - 2, rx + 2, ey);
  } else if (lvl == 1) {
    // round dot eyes
    u8g2.drawDisc(lx, ey - 1, 1);
    u8g2.drawDisc(rx, ey - 1, 1);
  } else if (lvl == 2) {
    // closed eyes: short horizontal lines
    u8g2.drawHLine(lx - 2, ey - 1, 5);
    u8g2.drawHLine(rx - 2, ey - 1, 5);
  } else {
    // x x eyes (mirrored)
    u8g2.drawLine(lx - 1, ey - 2, lx + 1, ey);
    u8g2.drawLine(lx - 1, ey, lx + 1, ey - 2);
    u8g2.drawLine(rx - 1, ey - 2, rx + 1, ey);
    u8g2.drawLine(rx - 1, ey, rx + 1, ey - 2);
  }

  // Nose (on the axis) + mouth (symmetric about the axis)
  uint8_t ny = hy + 12;
  u8g2.drawPixel(cx, ny);
  if (lvl == 0) {
    // smile
    u8g2.drawLine(cx - 3, ny + 1, cx, ny + 3);
    u8g2.drawLine(cx, ny + 3, cx + 3, ny + 1);
  } else if (lvl == 3) {
    // frown
    u8g2.drawLine(cx - 3, ny + 3, cx, ny + 1);
    u8g2.drawLine(cx, ny + 1, cx + 3, ny + 3);
  } else {
    // flat mouth, centered
    u8g2.drawHLine(cx - 2, ny + 2, 5);
  }
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

// PMS5003 frame: 0x42 0x4D | len(2) | 13x uint16 data | checksum(2).
// We read non-blocking; the sensor streams a frame roughly every ~1 s in
// active mode. Returns true when a valid checksummed frame was parsed.
bool readPms5003() {
  bool gotFrame = false;
  // Drain whatever is buffered this tick; keep only the freshest valid frame.
  while (PmsSerial.available() >= PMS_FRAME_LEN) {
    // Sync to start byte 0x42
    if (PmsSerial.peek() != 0x42) { PmsSerial.read(); continue; }

    uint8_t buf[PMS_FRAME_LEN];
    if (PmsSerial.readBytes(buf, PMS_FRAME_LEN) != PMS_FRAME_LEN) break;
    if (buf[1] != 0x4D) continue;   // second header byte must be 0x4D

    // Verify checksum: sum of bytes [0..29] == bytes[30..31]
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 30; i++) sum += buf[i];
    uint16_t chk = ((uint16_t)buf[30] << 8) | buf[31];
    if (sum != chk) continue;       // corrupt frame, skip

    // Atmospheric-environment concentrations (bytes 12..17)
    g_pm1  = ((uint16_t)buf[12] << 8) | buf[13];
    g_pm25 = ((uint16_t)buf[14] << 8) | buf[15];
    g_pm10 = ((uint16_t)buf[16] << 8) | buf[17];
    g_pmReady = true;
    gotFrame = true;
  }
  return gotFrame;
}

// =====================================================================
//  Display
// =====================================================================
void draw() {
  char buf[24];
  u8g2.clearBuffer();

  // ---- Header bar (white) with title + small status pill ----
  uint8_t overall = max(max(co2Level(g_co2), vocLevel(g_voc)), pm25Level(g_pm25));
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 14);

  u8g2.setFont(u8g2_font_logisoso16_tr);  // header in the logisoso family
  u8g2.setDrawColor(0);                    // black text on white bar
  u8g2.drawStr(3, 13, "ri's grade");

  // ---- Hero: bunny mascot + big status word ----
  u8g2.setDrawColor(1);
  drawBunny(2, 16, overall);

  const char* word = levelWord(overall);
  u8g2.setFont(u8g2_font_helvB14_tr);   // bold; "Xtreme" fits left of the value column
  u8g2.drawStr(28, 38, word);

  // ---- CO2 + PM2.5 as small labeled values on the right column ----
  u8g2.setFont(u8g2_font_5x8_tr);
  // CO2
  if (g_scdReady) snprintf(buf, sizeof(buf), "CO2 %u", g_co2);
  else            strcpy(buf, "CO2 --");
  u8g2.drawStr(127 - u8g2.getStrWidth(buf), 25, buf);
  // PM2.5
  if (g_pmReady)  snprintf(buf, sizeof(buf), "PM %u", g_pm25);
  else            strcpy(buf, "PM --");
  u8g2.drawStr(127 - u8g2.getStrWidth(buf), 35, buf);

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

  // PMS5003 on UART1 (active mode by default - it just streams frames)
  PmsSerial.begin(PMS_BAUD, SERIAL_8N1, PMS_RX, PMS_TX);

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
    readPms5003(); // 4) PM1.0 / PM2.5 / PM10 (UART stream)

    draw();

    Serial.printf("CO2=%u ppm  VOC=%ld  T=%.2f C  RH=%.1f %%  "
                  "PM1=%u PM2.5=%u PM10=%u ug/m3\n",
                  g_co2, (long)g_voc, g_temp, g_rh,
                  g_pm1, g_pm25, g_pm10);
  }
}
