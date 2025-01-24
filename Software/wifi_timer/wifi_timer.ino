#include <WiFi.h>
#include <NTPClient.h>
#include <FastLED.h>

#define DATA_PIN 27
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

const char* ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);

void setup() {
  Serial.begin(115200);
  while(!Serial);

  String ssid = "";
  String pass = "";

  bool readSplit = false;

  while (1) {
    if (Serial.available()) {
      while (Serial.available() > 0) {
        char c = Serial.read();
        if (readSplit == false && c == '*') {
          readSplit = true;
          continue;
        }
        if (readSplit) {
          pass += c;
        } else {
          ssid += c;
        }
      }
      break;
    }
  }
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  timeClient.begin();

  leds[0] = 0xFF0f0FF;
  FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.show();
}

unsigned char txBuff[3];

void loop() {
  timeClient.update();
 
  unsigned long epochTime = timeClient.getEpochTime();
  if (epochTime < 10000) {
    delay(500);
    return;
  }

  unsigned char hour = timeClient.getHours() + 8;
  unsigned char minu = timeClient.getMinutes();
  unsigned char sec = timeClient.getSeconds();
  txBuff[0] = 0xC0 | hour;
  txBuff[1] = 0x80 | minu;
  txBuff[2] = 0x40 | sec;
  Serial.write(txBuff, 3);
  delay(5000);
}
