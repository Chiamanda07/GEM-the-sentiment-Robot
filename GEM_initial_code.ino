// ============================================================
//  AI Robot Firmware — ESP32-S3
//  Hardware: INMP441 | MAX98357A | SSD1306 OLED | Gemini API
// ============================================================
//  Libraries needed (install via Arduino Library Manager):
//    - TFT_eSPI  (configure User_Setup.h for GC9A01 + your pins)
//    - ArduinoJson
//    - WiFi (built-in ESP32)
//    - HTTPClient (built-in ESP32)
// ============================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <driver/i2s.h>
#include <Base64.h>

// ---------- WiFi & API Config ----------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* GEMINI_API_KEY = "YOUR_GEMINI_API_KEY";
const char* GEMINI_URL    = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent";
const char* EMOTION_API   = "http://YOUR_PC_IP:5000/emotion"; // <-- from emotion_server.py output

// ---------- Pin Definitions ----------
// INMP441 Microphone (I2S)
#define MIC_WS    42
#define MIC_SCK   41
#define MIC_SD    40
#define I2S_MIC_PORT  I2S_NUM_0

// MAX98357A Speaker (I2S)
#define SPK_BCLK  38
#define SPK_LRC   37
#define SPK_DIN   36
#define I2S_SPK_PORT  I2S_NUM_1

// TFT Round Display (GC9A01, 240x240) — pins set in TFT_eSPI User_Setup.h
#define SCREEN_W  240
#define SCREEN_H  240
#define CX        120   // center x
#define CY        120   // center y

// Button to trigger recording
#define BTN_PIN   0   // Use BOOT button or wire your own

// ---------- Audio Config ----------
#define SAMPLE_RATE     16000
#define RECORD_SECONDS  4
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define AUDIO_BUF_SIZE  (SAMPLE_RATE * RECORD_SECONDS * 2) // 16-bit = 2 bytes

// ---------- Globals ----------
TFT_eSPI tft = TFT_eSPI();
int8_t* audioBuf = nullptr;

// ---------- Emotion Faces (GC9A01 240x240 Round TFT) ----------
void drawFace(const String& emotion) {
  tft.fillScreen(TFT_BLACK);

  uint16_t faceColor = TFT_BLUE;
  uint16_t detailColor = TFT_BLACK;

  // Face circle
  tft.fillCircle(CX, CY, 100, faceColor);
  tft.drawCircle(CX, CY, 100, TFT_WHITE);

  if (emotion == "HAPPY") {
    // Eyes
    tft.fillCircle(CX - 30, CY - 25, 10, detailColor);
    tft.fillCircle(CX + 30, CY - 25, 10, detailColor);
    // Rosy cheeks
    tft.fillCircle(CX - 50, CY + 10, 12, TFT_RED);
    tft.fillCircle(CX + 50, CY + 10, 12, TFT_RED);
    // Big smile arc
    for (int i = -40; i <= 40; i++) {
      int x = CX + i;
      int y = CY + 30 + (i * i) / 80;
      tft.drawPixel(x, y,     TFT_BLACK);
      tft.drawPixel(x, y + 1, TFT_BLACK);
      tft.drawPixel(x, y + 2, TFT_BLACK);
    }

  } else if (emotion == "SAD") {
    // Eyes with tears
    tft.fillCircle(CX - 30, CY - 25, 10, detailColor);
    tft.fillCircle(CX + 30, CY - 25, 10, detailColor);
    tft.fillCircle(CX - 30, CY - 5, 5, TFT_CYAN); // tear
    tft.fillCircle(CX + 30, CY - 5, 5, TFT_CYAN);
    // Frown
    for (int i = -40; i <= 40; i++) {
      int x = CX + i;
      int y = CY + 55 - (i * i) / 80;
      tft.drawPixel(x, y,     TFT_BLACK);
      tft.drawPixel(x, y + 1, TFT_BLACK);
      tft.drawPixel(x, y + 2, TFT_BLACK);
    }

  } else if (emotion == "THINKING") {
    // One eye open, one squinted
    tft.fillCircle(CX - 30, CY - 25, 10, detailColor);
    tft.fillRect(CX + 18, CY - 28, 26, 5, detailColor); // squint line
    // Flat mouth
    tft.fillRect(CX - 30, CY + 35, 60, 5, detailColor);
    // Thought bubbles
    tft.fillCircle(CX + 75, CY - 60, 6,  TFT_WHITE);
    tft.fillCircle(CX + 85, CY - 75, 9,  TFT_WHITE);
    tft.fillCircle(CX + 95, CY - 92, 13, TFT_WHITE);

  } else if (emotion == "SURPRISED") {
    // Wide eyes
    tft.drawCircle(CX - 30, CY - 25, 16, detailColor);
    tft.drawCircle(CX - 30, CY - 25, 15, detailColor);
    tft.fillCircle(CX - 30, CY - 25, 8,  detailColor);
    tft.drawCircle(CX + 30, CY - 25, 16, detailColor);
    tft.drawCircle(CX + 30, CY - 25, 15, detailColor);
    tft.fillCircle(CX + 30, CY - 25, 8,  detailColor);
    // Open mouth (oval)
    tft.fillEllipse(CX, CY + 40, 22, 28, detailColor);
    tft.fillEllipse(CX, CY + 42, 16, 20, TFT_RED);

  } else {
    // NEUTRAL
    tft.fillCircle(CX - 30, CY - 25, 10, detailColor);
    tft.fillCircle(CX + 30, CY - 25, 10, detailColor);
    tft.fillRect(CX - 30, CY + 35, 60, 5, detailColor);
  }
}

void showText(const String& line1, const String& line2 = "") {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, line2 == "" ? 110 : 95);
  tft.println(line1);
  if (line2 != "") {
    tft.setCursor(20, 125);
    tft.println(line2);
  }
}

// ---------- I2S Microphone Setup ----------
void setupMic() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
  };
  i2s_pin_config_t pins = {
    .bck_io_num = MIC_SCK,
    .ws_io_num  = MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = MIC_SD,
  };
  i2s_driver_install(I2S_MIC_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_MIC_PORT, &pins);
}

// ---------- I2S Speaker Setup ----------
void setupSpeaker() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
  };
  i2s_pin_config_t pins = {
    .bck_io_num = SPK_BCLK,
    .ws_io_num  = SPK_LRC,
    .data_out_num = SPK_DIN,
    .data_in_num  = I2S_PIN_NO_CHANGE,
  };
  i2s_driver_install(I2S_SPK_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_SPK_PORT, &pins);
}

// ---------- Record Audio ----------
bool recordAudio() {
  showText("Listening...");
  Serial.println("Recording...");

  size_t bytesRead = 0, total = 0;
  while (total < AUDIO_BUF_SIZE) {
    i2s_read(I2S_MIC_PORT, audioBuf + total,
             min((size_t)512, AUDIO_BUF_SIZE - total),
             &bytesRead, portMAX_DELAY);
    total += bytesRead;
  }

  Serial.printf("Recorded %d bytes\n", total);
  return true;
}

// ---------- Send to Gemini & Get Response ----------
String callGemini() {
  showText("Thinking...");
  drawFace("THINKING");

  // Build WAV header (44 bytes) + raw PCM
  int dataSize = AUDIO_BUF_SIZE;
  int wavSize  = dataSize + 44;
  uint8_t* wav = (uint8_t*)malloc(wavSize);
  if (!wav) { Serial.println("OOM"); return ""; }

  // WAV header
  memcpy(wav,      "RIFF", 4);
  uint32_t chunkSize = wavSize - 8;
  memcpy(wav + 4,  &chunkSize, 4);
  memcpy(wav + 8,  "WAVE", 4);
  memcpy(wav + 12, "fmt ", 4);
  uint32_t subchunk1 = 16; memcpy(wav + 16, &subchunk1, 4);
  uint16_t audioFmt  = 1;  memcpy(wav + 20, &audioFmt, 2);
  uint16_t numCh     = 1;  memcpy(wav + 22, &numCh, 2);
  uint32_t sr        = SAMPLE_RATE; memcpy(wav + 24, &sr, 4);
  uint32_t byteRate  = SAMPLE_RATE * 2; memcpy(wav + 28, &byteRate, 4);
  uint16_t blockAlign = 2; memcpy(wav + 32, &blockAlign, 2);
  uint16_t bps        = 16; memcpy(wav + 34, &bps, 2);
  memcpy(wav + 36, "data", 4);
  memcpy(wav + 40, &dataSize, 4);
  memcpy(wav + 44, audioBuf, dataSize);

  // Base64 encode
  String b64 = base64::encode(wav, wavSize);
  free(wav);

  // Build JSON payload
  String payload = "{\"contents\":[{\"parts\":["
    "{\"text\":\"You are a friendly robot assistant. Always start your reply with an emotion tag: [HAPPY], [SAD], [THINKING], [SURPRISED], or [NEUTRAL]. Then respond conversationally in 1-2 sentences.\"},"
    "{\"inline_data\":{\"mime_type\":\"audio/wav\",\"data\":\"" + b64 + "\"}}"
    "]}]}";

  HTTPClient http;
  String url = String(GEMINI_URL) + "?key=" + GEMINI_API_KEY;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(15000);

  int code = http.POST(payload);
  String result = "";

  if (code == 200) {
    String body = http.getString();
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, body) == DeserializationError::Ok) {
      result = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
    Serial.println(http.getString());
    result = "[NEUTRAL] Sorry, I couldn't connect.";
  }

  http.end();
  return result;
}

// ---------- Get Emotion from Your Pickle Model API ----------
String getEmotion(const String& text) {
  if (WiFi.status() != WL_CONNECTED) return "NEUTRAL";

  HTTPClient http;
  http.begin(EMOTION_API);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

  String payload = "{\"text\":\"" + text + "\"}";
  int code = http.POST(payload);
  String emotion = "NEUTRAL";

  if (code == 200) {
    String body = http.getString();
    DynamicJsonDocument doc(256);
    if (deserializeJson(doc, body) == DeserializationError::Ok) {
      emotion = doc["emotion"].as<String>();
      Serial.println("Model raw output: " + doc["raw"].as<String>());
    }
  } else {
    Serial.printf("Emotion API error: %d\n", code);
  }

  http.end();
  return emotion;
}

// ---------- Simple TTS via Speaker ----------
// NOTE: For real speech output, replace this with a TTS API
// (e.g. Google TTS, ElevenLabs) that returns audio bytes.
// This function just prints to Serial and shows text on OLED
// as a placeholder until TTS is wired up.
void speak(const String& text) {
  Serial.println("Robot says: " + text);

  // Show response text on OLED (scrolling in 20-char chunks)
  int len = text.length(), i = 0;
  while (i < len) {
    String chunk = text.substring(i, min(i + 20, len));
    showText("Robot:", chunk);
    delay(2000);
    i += 20;
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // TFT init
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  showText("Booting...");

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  showText("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
  showText("WiFi OK!", WiFi.localIP().toString());
  delay(1000);

  // I2S
  setupMic();
  setupSpeaker();

  // Alloc audio buffer in PSRAM if available
  audioBuf = (int8_t*)(ESP.getPsramSize() > 0
    ? ps_malloc(AUDIO_BUF_SIZE)
    : malloc(AUDIO_BUF_SIZE));
  if (!audioBuf) { Serial.println("Audio buffer alloc failed!"); while(true); }

  drawFace("HAPPY");
  delay(1000);
  drawFace("NEUTRAL");
  showText("Hold BTN to talk");
}

// ---------- Main Loop ----------
void loop() {
  // Wait for button press
  if (digitalRead(BTN_PIN) == LOW) {
    delay(50); // debounce
    if (digitalRead(BTN_PIN) == LOW) {

      // 1. Record
      recordAudio();

      // 2. Send to Gemini
      String response = callGemini();
      if (response == "") {
        drawFace("SAD");
        showText("Error :(");
        delay(2000);
        drawFace("NEUTRAL");
        return;
      }

      // 3. Get emotion from your trained pickle model
      Serial.println("Response: " + response);
      String emotion = getEmotion(response);
      Serial.println("Emotion: " + emotion);
      drawFace(emotion);

      // 4. Speak response
      speak(response);

      // 5. Return to idle
      delay(1000);
      drawFace("NEUTRAL");
      showText("Hold BTN to talk");

      // Wait for button release
      while (digitalRead(BTN_PIN) == LOW) delay(10);
    }
  }
}