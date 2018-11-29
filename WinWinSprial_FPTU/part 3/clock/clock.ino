/*

  Clock.ino

  Demonstrates a clock-like display with a very huge font.
  Also shows usage of u8x8_u8toa().

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include "mpu.h"
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <Wire.h>
#include "frame.h"
#include <DS3231.h>


DS3231 clock;
RTCDateTime dt;



/*
  U8glib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.

  This is a page buffer example.
*/

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

int16_t pinTouch[] = {21, 22, 23, 24, 20, 18, 17, 16, 13, 12};

int pre = -1;
char days[20][20] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};
int count = 0;
unsigned long time_now = 0;
unsigned long time_now2 = 0;
unsigned long time_now_tem = 0;
#define LED_PIN 13
bool blinkState = false;
int threshold = 20;
bool touch1detected = false;
bool touch2detected = false;

void gotTouch1() {
  touch1detected = true;
}

void gotTouch2() {
  touch2detected = true;
}
const char *string_list =
  "acel/guro demo\n"
  "Infrared demo\n"
  "Heart-rate demo\n"
  "Setting\n"
  "About\n";

uint8_t current_selection = 2;

void setup(void) {

  // u8g2.begin();
  // u8g2.begin(/*Select=*/ 0, /*Right/Next=*/ 1, /*Left/Prev=*/ 2, /*Up=*/ 4, /*Down=*/ 3, /*Home/Cancel=*/ A6);
  u8g2.begin(/*Select=*/ U8X8_PIN_NONE, /*Right/Next=*/ U8X8_PIN_NONE, /*Left/Prev=*/ U8X8_PIN_NONE, /*Up=*/ U8X8_PIN_NONE, /*Down=*/ 4, /*Home/Cancel=*/ 2);
  setup_mpu();
  u8g2.enableUTF8Print();
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  // Initialize DS3231
  Serial.println("Initialize DS3231");
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);
}


void showTime() {

  dt = clock.getDateTime();
  if (millis() > (time_now + 500) ) {
    time_now = millis();
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_logisoso42_tn);
      u8g2.setCursor(0, 42);
      if (dt.hour < 10)
        u8g2.print("0");
      u8g2.print(dt.hour);

      u8g2.setCursor(75 , 42);
      if (dt.minute < 10)
        u8g2.print("0");
      u8g2.print(dt.minute );

      u8g2.setFont(u8g2_font_timB18_tr);

      u8g2.setCursor(60, 20);
      if (dt.second % 2 == 0) {
        u8g2.print(":");
      }
      u8g2.setCursor(55, 42);
      if (dt.second < 10)
        u8g2.print("0");
      u8g2.print(dt.second );

      u8g2.setFont(u8g2_font_timB10_tf);

    } while ( u8g2.nextPage() );

  }
}

void showTime2() {

  dt = clock.getDateTime();
  if (millis() > (time_now2 + 500) ) {
    time_now2 = millis();
    char _time[20];
    char _date[20];
    char _year[6];
    if (dt.second % 2 == 0) {
      strcpy(_time, u8x8_u8toa(dt.hour, 2));
      strcat(_time, " : ");
      strcat(_time, u8x8_u8toa(dt.minute, 2));
    } else {
      strcpy(_time, u8x8_u8toa(dt.hour, 2));
      strcat(_time, "   ");
      strcat(_time, u8x8_u8toa(dt.minute, 2));
    }
    sprintf(_year, "%d", dt.year);
    strcpy(_date, u8x8_u8toa(dt.day, 2));
    strcat(_date, "-");
    strcat(_date, u8x8_u8toa(dt.month, 2));
    strcat(_date, "-");
    strcat(_date, _year);

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB18_tf);
      u8g2.drawStr(25, 33, _time);
      u8g2.setFont(u8g2_font_timB10_tf);
      u8g2.drawStr(31, 52, _date);

    } while ( u8g2.nextPage() );
  }
}

int touch() {
  blinkState = !blinkState;
  digitalWrite(LED_PIN, blinkState);

  int decision = 0;

  int check1 = touchRead(15);
  int check2 = touchRead(4);
  if (check1 < 60 && check1 > 20) {
    decision = -1;
    Serial.print("check1:");
    //    Serial.println(check1);

  }
  if (check2 < 80 && check2 > 0) {
    decision = 1;
    Serial.print("check2:");
    //    Serial.println(check2);

  }

  if (decision > 0) {
    Serial.println("right");
    //    showTem();

  }
  if (decision < 0) {
    Serial.println("left");
    //    showTime();

  }
  Serial.println(decision);

}


void showMPU() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_timB10_tf);
    u8g2.setCursor(0, 42);
    u8g2.print("acl:");
    u8g2.print(axx());
    u8g2.print(" ");
    u8g2.print(ayy());
    u8g2.print(" ");
    u8g2.print(azz());
  } while ( u8g2.nextPage() );

  Serial.println(get_acelgyro_json());
}

void loop(void) {

  u8g2.setFont(u8g2_font_6x12_tr);
  current_selection = u8g2.userInterfaceSelectionList(
                        "Menu",
                        current_selection,
                        string_list);
  if (current_selection == 2) {
    showMPU();
    select = 3;
  }

}
