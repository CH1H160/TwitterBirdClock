/*
  AudioFileSourceVoiceTextStream
  https://github.com/robo8080/M5Core2_Avatar_VoiceText_TTS
  
  Copyright (C) 2021  robo8080

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * BirdClock
 * Copyright (C) 2022  Ryota Kobayashi
 * This program is created by citing the above software and inheriting the library.
 */

#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>
#include <M5Core2.h>
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourceVoiceTextStream.h"

#define JST (3600L * 9)

TFT_eSprite sprite = TFT_eSprite(&M5.Lcd);
const char *SSID = "SSID";
const char *PASSWORD = "PASSWORD";
const char* AUTH = "AUTH";
Servo servo;

AudioGeneratorMP3 *mp3;
AudioFileSourceVoiceTextStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2S *out;
const int preallocateBufferSize = 40*1024;
uint8_t *preallocateBuffer;

const uint8_t PIN_SERVO = 19;

char *tts_parms ="&emotion=happiness&format=mp3&speaker=hikari&volume=200&speed=120&pitch=130";

void StatusCallback(void *cbData, int code, const char *string){
  const char *ptr = reinterpret_cast<const char *>(cbData);
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
}

void VoiceText_tts(char *text,char *tts_parms) {
  file = new AudioFileSourceVoiceTextStream( text, tts_parms);
  buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
  delay(100);
  mp3->begin(buff, out);
}

void setup(){
  M5.begin(true, false, true);
  M5.Axp.SetSpkEnable(true);
  
  WiFi.mode(WIFI_STA);
  Blynk.begin(AUTH, SSID, PASSWORD);

  configTime(JST, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
  sprite.setColorDepth(8);
  sprite.createSprite(M5.Lcd.width(), M5.Lcd.height());
  sprite.setTextColor(CYAN);
  
  servo.setPeriodHertz(50);
  servo.attach(PIN_SERVO, 700, 2300);
  servo.write(80);
  
  preallocateBuffer = (uint8_t*)ps_malloc(preallocateBufferSize);
  out = new AudioOutputI2S();
  out->SetPinout(12, 0, 2);
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");

  xTaskCreatePinnedToCore(clock_func, "clock_func", 4096, NULL, 1, NULL, 1);
}

void loop(){
  Blynk.run();
  if (mp3->isRunning()) {
    mp3->loop();
  }
}

BLYNK_WRITE(V1){
  String pinValue = param.asStr();
  char text[pinValue.length() + 1];
  pinValue.toCharArray(text, pinValue.length() + 1);

  VoiceText_tts(text, tts_parms);
  
  servo.write(0);
  delay(500);
  servo.write(80);
}

void clock_func(void* arg) {
  struct tm tm;
  while (1) {
    if(getLocalTime(&tm)){
      sprite.fillScreen(BLACK);
      sprite.setCursor(20,30);
      sprite.setTextSize(3);
      sprite.printf("TwitterBirdClock");
      sprite.setCursor(20,90);
      sprite.setTextSize(6);
      sprite.printf("%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
      sprite.setCursor(40,170);
      sprite.setTextSize(4);
      sprite.printf("%d/%2d/%2d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
      sprite.pushSprite(0, 0);  
    }
  }
}
