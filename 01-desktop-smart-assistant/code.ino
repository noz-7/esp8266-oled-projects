#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi Bilgileri
const char* ssid     = "WIFI_ADI";       // Kendi Wi-Fi adını yaz
const char* password = "WIFI_SIFRESI";   // Kendi Wi-Fi şifreni yaz

#define FLASH_BUTTON 0

// Ekran Ayarları (128x64: Üst 16px Sarı, Alt 48px Mavi)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NTP Saat Ayarı (GMT+3)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 60000);

// Değişkenler
float usdRate = 0.0, eurRate = 0.0, gbpRate = 0.0;
float tempNicosia = 0.0;
int currentMode = 0;
bool isFrozen = false;

unsigned long lastSwitchTime = 0;
unsigned long lastFetchTime = 0;
unsigned long buttonPressStart = 0;
bool buttonStatePrevious = HIGH;

struct Course {
  int hour;
  String timeRoom;
  String code;
};

void updateData() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  // 1. Döviz Verileri
  if (http.begin(client, "https://open.er-api.com/v6/latest/USD")) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getString());
      usdRate = doc["rates"]["TRY"];
      float eurBase = doc["rates"]["EUR"];
      float gbpBase = doc["rates"]["GBP"];
      if (eurBase > 0) eurRate = usdRate / eurBase;
      if (gbpBase > 0) gbpRate = usdRate / gbpBase;
    }
    http.end();
  }

  // 2. Hava Durumu
  if (http.begin(client, "https://api.open-meteo.com/v1/forecast?latitude=35.1853&longitude=33.3642&current_weather=true")) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, http.getString());
      tempNicosia = doc["current_weather"]["temperature"];
    }
    http.end();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FLASH_BUTTON, INPUT_PULLUP);

  Wire.begin(12, 14);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("Wi-Fi Baglaniyor...");
  display.display();

  WiFi.begin(ssid, password);
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    display.print(".");
    display.display();
    retryCount++;
  }

  timeClient.begin();
  timeClient.update();
  updateData();
  lastFetchTime = millis();
}

// 16x24 Piksel Özel Euro Simgesi (€) Çizimi
void drawEuroSymbolCustom(int x, int y) {
  display.drawRoundRect(x, y, 16, 24, 6, SSD1306_WHITE);
  display.fillRect(x + 8, y, 10, 24, SSD1306_BLACK); // Arka C şekli
  display.drawFastHLine(x - 2, y + 8, 14, SSD1306_WHITE);
  display.drawFastHLine(x - 2, y + 14, 14, SSD1306_WHITE);
}

// 16x24 Piksel Özel Sterlin Simgesi (£) Çizimi
void drawPoundSymbolCustom(int x, int y) {
  display.setTextSize(3);
  display.setCursor(x, y);
  display.print("L");
  display.drawFastHLine(x - 2, y + 11, 12, SSD1306_WHITE);
  display.drawFastHLine(x - 2, y + 20, 16, SSD1306_WHITE);
}

void drawFreezeIndicator() {
  if (isFrozen) {
    display.setTextSize(1);
    display.setCursor(110, 0);
    display.print("[*]");
  }
}

// ----- EKRAN SAYFALARI -----

// 1. DEV SAAT (Sıfır Kenar Boşluğu)
void showPageClockTime() {
  String shortTime = timeClient.getFormattedTime().substring(0, 5);
  display.setTextSize(4);
  display.setCursor(4, 20); // Boşluksuz mavi ekrana sığar
  display.println(shortTime);
}

// 2. DEV GÜN (Kısa Günler Dev, Uzun Günler Tam Ortalı)
void showPageClockDay() {
  String days[] = {"PAZAR", "PAZARTESI", "SALI", "CARSAMBA", "PERSEMBE", "CUMA", "CUMARTESI"};
  String currentDay = days[timeClient.getDay()];

  if (currentDay.length() <= 5) { // CUMA, SALI, PAZAR
    display.setTextSize(3);
    int xPos = (128 - (currentDay.length() * 18)) / 2;
    display.setCursor(xPos > 0 ? xPos : 2, 22);
  } else { // PAZARTESI, CARSAMBA, PERSEMBE, CUMARTESI
    display.setTextSize(2);
    int xPos = (128 - (currentDay.length() * 12)) / 2;
    display.setCursor(xPos > 0 ? xPos : 0, 26);
  }
  display.println(currentDay);
}

// 3. DOLAR
void showPageUSD() {
  display.setTextSize(4);
  display.setCursor(0, 20);
  display.print("$");
  display.setTextSize(3);
  display.setCursor(30, 24);
  display.println(usdRate, 1);
}

// 4. EURO (Piksel Özel € Simgesi)
void showPageEUR() {
  drawEuroSymbolCustom(5, 22);
  display.setTextSize(3);
  display.setCursor(32, 24);
  display.println(eurRate, 1);
}

// 5. STERLİN (£ Simgesi)
void showPageGBP() {
  drawPoundSymbolCustom(5, 22);
  display.setTextSize(3);
  display.setCursor(32, 24);
  display.println(gbpRate, 1);
}

// 6. LEFKOŞA (Mavi Ekranı Tam Dolduran Dev Font)
void showPageCity() {
  display.setTextSize(3);
  display.setCursor(2, 22); // Tam genişlik
  display.println("LEFKOSA");
}

// 7. DERECE (°C Simgeli)
void showPageTemp() {
  display.setTextSize(4);
  display.setCursor(10, 20);
  display.print((int)tempNicosia);

  display.drawCircle(80, 24, 3, SSD1306_WHITE);
  display.setTextSize(3);
  display.setCursor(90, 22);
  display.println("C");
}

// 8. HAVA TAVSİYESİ
void showPageWeatherAdvice() {
  display.setTextSize(1);
  display.setCursor(18, 4);
  display.println("HAVA TAVSIYESI");

  display.setTextSize(2);
  display.setCursor(5, 28);
  if (tempNicosia >= 28.0) display.println("GUNES KREMI");
  else if (tempNicosia >= 15.0) display.println("ILIK HAVA");
  else if (tempNicosia >= 10.0) display.println("MONTUNU AL");
  else display.println("SEMSIYE AL");
}

// 9. NOZ-7 (ÜST SARI ŞERİT FULL DOLU, ALT MAVI TERS RENK)
void showPageNickname() {
  // Üst Sarı Şeridi Düz Full Yakar
  display.fillRect(0, 0, 128, 16, SSD1306_WHITE);

  // Alt Mavi Alanı da Düz Yakar (Ters Renk)
  display.fillRect(0, 16, 128, 48, SSD1306_WHITE);

  // Mavi Ekran Ortasına Siyah NOZ-7 Yazar
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(4);
  display.setCursor(4, 22);
  display.println("NOZ-7");

  display.setTextColor(SSD1306_WHITE); // Rengi normale döndür
}

// 10. AKILLI DERS PROGRAMI
void showPageSchedule() {
  int day = timeClient.getDay();
  int currentHour = timeClient.getHours();

  Course todayCourses[8];
  int count = 0;

  if (day == 1) { // Pazartesi
    todayCourses[0] = {14, "14.00 - EC201", "CMP412"};
    todayCourses[1] = {15, "15.00 - EC201", "CMP412"};
    count = 2;
  } else if (day == 2) { // Salı
    todayCourses[0] = {12, "12.00 - GE120", "CMP332"};
    todayCourses[1] = {13, "13.00 - ST107", "CMP412"};
    todayCourses[2] = {14, "14.00 - ST203", "CMP415"};
    count = 3;
  } else if (day == 3) { // Çarşamba
    todayCourses[0] = {9,  "09.00 - ST103", "CMP415"};
    todayCourses[1] = {11, "11.00 - ST263", "SWN301"};
    todayCourses[2] = {13, "13.00 - GE121", "CMP412"};
    todayCourses[3] = {15, "15.00 - GE121", "CMP332"};
    count = 4;
  } else if (day == 4) { // Perşembe
    todayCourses[0] = {9,  "09.00 - ST232", "SWN301"};
    todayCourses[1] = {12, "12.00 - ST113", "CMP415"};
    todayCourses[2] = {14, "14.00 - GE202", "CMP242"};
    count = 3;
  } else if (day == 5) { // Cuma
    todayCourses[0] = {9,  "09.00 - ST235", "CMP242"};
    todayCourses[1] = {13, "13.00 - ST105", "CMP332"};
    todayCourses[2] = {14, "14.00 - ST235", "CMP242"};
    todayCourses[3] = {15, "15.00 - GE211", "SWN301"};
    todayCourses[4] = {17, "17.00 - ST235", "CMP415"};
    count = 5;
  }

  int foundIndex = -1;
  for (int i = 0; i < count; i++) {
    if (todayCourses[i].hour >= currentHour) {
      foundIndex = i;
      break;
    }
  }

  if (foundIndex != -1) {
    display.setTextSize(1);
    String headerText = todayCourses[foundIndex].timeRoom;
    int xPos = (128 - (headerText.length() * 6)) / 2;
    display.setCursor(xPos > 0 ? xPos : 0, 4);
    display.println(headerText);

    display.setTextSize(3);
    display.setCursor(10, 24);
    display.println(todayCourses[foundIndex].code);
  } else {
    display.setTextSize(1);
    display.setCursor(30, 4);
    display.println("DERS DURUMU");

    display.setTextSize(2);
    display.setCursor(10, 28);
    if (count == 0) display.println("DERS YOK");
    else display.println("DERS BTTI");
  }
}

// Tek Tıkla Sabitleme
void checkButton() {
  bool currentReading = digitalRead(FLASH_BUTTON);

  if (buttonStatePrevious == HIGH && currentReading == LOW) {
    buttonPressStart = millis();
  }
  else if (buttonStatePrevious == LOW && currentReading == HIGH) {
    long duration = millis() - buttonPressStart;
    if (duration > 50 && duration < 800) {
      isFrozen = !isFrozen;
    }
  }
  buttonStatePrevious = currentReading;
}

void loop() {
  timeClient.update();
  checkButton();

  // 10 dakikada bir veri güncelleme
  if (millis() - lastFetchTime > 600000) {
    updateData();
    lastFetchTime = millis();
  }

  // 4 saniyede bir ekran değiştir
  if (!isFrozen && (millis() - lastSwitchTime > 4000)) {
    currentMode = (currentMode + 1) % 10;
    lastSwitchTime = millis();
  }

  display.clearDisplay();

  switch (currentMode) {
    case 0: showPageClockTime(); break;
    case 1: showPageClockDay(); break;
    case 2: showPageUSD(); break;
    case 3: showPageEUR(); break;
    case 4: showPageGBP(); break;
    case 5: showPageCity(); break;
    case 6: showPageTemp(); break;
    case 7: showPageWeatherAdvice(); break;
    case 8: showPageNickname(); break;
    case 9: showPageSchedule(); break;
  }

  drawFreezeIndicator();
  display.display();
  delay(50);
}
