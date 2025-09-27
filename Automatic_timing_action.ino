#include <WiFi.h>
#include <time.h>
#include <ESP32Servo.h>

// ===== WiFi 設定 =====
const char* ssid = "";
const char* password = "";

// ===== 伺服馬達 =====
Servo myServo;
const int servoPin = 16; // 連接伺服的腳位

// ===== 多時間點管理 =====
struct TriggerTime {
  int hour;
  int minute;
  bool triggered;  // 當前分鐘是否已觸發
};

#define MAX_TRIGGERS 10
TriggerTime triggerTimes[MAX_TRIGGERS] = {
  {7, 55, false},
  {12, 5, false},
  // {17, 1, false},
  // {17, 2, false},
  // {17, 4, false},
  // {17, 5, false},
  {17, 0, false}
};
int triggerCount = 3; // 實際使用的時間點數

// ===== NTP 設定 =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600; // GMT+8
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi connected!");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
    } else {
      Serial.printf("Time synchronized: %02d:%02d:%02d\n", 
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disconnected, running offline now.");
  } else {
    Serial.println("\nFailed to connect WiFi. Using internal RTC time.");
  }
}

void loop() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to read time from RTC");
    delay(1000);
    return;
  }

  int h = timeinfo.tm_hour;
  int m = timeinfo.tm_min;
  Serial.printf("Now time is %02d:%02d!\n", h, m);
  for(int i=0; i<triggerCount; i++){
    if(h == triggerTimes[i].hour && m == triggerTimes[i].minute){
      if(!triggerTimes[i].triggered){ 
        Serial.printf("Trigger servo at %02d:%02d!\n", h, m);
        myServo.attach(servoPin);
        myServo.write(165);
        delay(1000);
        myServo.write(15);
        delay(1000);
        myServo.write(165);
        delay(1000);
        myServo.write(15);
        delay(1000);
        myServo.write(165);
        delay(1000);
        myServo.write(15);
        delay(1000);
        myServo.detach(); 
        triggerTimes[i].triggered = true;
      }
    } else {
      triggerTimes[i].triggered = false;
    }
  }

  delay(15000); // 每15秒檢查一次
}
