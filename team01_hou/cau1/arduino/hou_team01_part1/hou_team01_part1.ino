/*
   FFC Team
   FITHOU - VN
   Main sketch
*/

/* main sketch file: global include for other files */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <time.h>
#include <math.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_deep_sleep.h>
#include <soc/rtc.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <driver/gpio.h>

#include <U8g2lib.h>
#include <SimpleTimer.h>
// ESP32 version (https://github.com/zhouhan0126/WIFIMANAGER-ESP32)
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <RTClib.h>

/*
#include <menu.h>
#include <menuIO/u8g2Out.h>
// #include <menuIO/encoderIn.h>
// #include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
//#include <PubSubClient.h>
*/

#define SCREEN_W 128
#define SCREEN_H 64
#define BAUD 115200
#define WIFI_SSID "PTIT_LAB DAO TAO"
#define WIFI_PSK "buuchinh@"


#define TOUCH_UP_PIN 14
#define TOUCH_CENTER_PIN 12
#define TOUCH_DOWN_PIN 13
// Qua không khí thì touch threshold thường là 100-120, khi chạm gần là 5-15, kết nối trực tiếp là 1
#define TOUCH_THRESHOLD 16
// tối đa 1200ms chỉ nhận 1 lần touch
#define TOUCH_MAXIMUM_TIME 1200l
// chạm tối thiếu 150ms để nhận touch
#define TOUCH_MINIMUM_TIME 250l

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
SimpleTimer stimer;
RTC_DS3231 rtc;

// 0 - clock
// 1 - sensor
int displaying = 0;

char ntp_server[101];
int timer[3] = { -1, -1, -1};
int timezone = +7;
int dst = 0;

const uint8_t* clock_font = u8g2_font_7x14B_mr;
const uint8_t* clock_font_small = u8g2_font_logisoso26_tn;

/*
void func (void) {
}

char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";

const colorDef<uint8_t> menu_colors[] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};


serialIn serial(Serial);

MENU_INPUTS(in,&serial);
MENU_OUTPUTS(out,2,U8G2_OUT(u8g2, menu_colors, 5, 9, 0, 0, {0, 0 , SCREEN_W/5, SCREEN_H/9}), SERIAL_OUT(Serial));

MENU(mainMenu,"ESP32 Clock",doNothing,noEvent,wrapStyle
  ,OP("Op1",doNothing,noEvent)
  ,OP("Op2",doNothing,noEvent)
//  ,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(time)
  ,SUBMENU(func)
  ,SUBMENU(func)
  ,OP("LED On",func,enterEvent)
  ,OP("LED Off",func,enterEvent)
  ,SUBMENU(func)
  ,SUBMENU(func)
  ,OP("Alert test",func,enterEvent)
//  ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  ,EXIT("<Exit")
);

MENU(subMenu,"Sub-Menu",doNothing,noEvent,noStyle
  ,OP("Sub1",func,noEvent)
  //,altOP(altPrompt,"",doNothing,noEvent)
  ,EXIT("<Back")
);

NAVROOT(nav, mainMenu, 2, in, out);

*/

unsigned long button_hdl_temp;
unsigned long button_hdl_presstime = 0l;
uint8_t button_hdl_press = 0;
bool button_hdl_pressed = false;

bool button_begin_proc (uint8_t num) {
  if (button_hdl_press != num) {
    Serial.printf("key%d: pressing\n", num);
    button_hdl_presstime = millis();
    button_hdl_pressed = false;
    button_hdl_press = num;
  }
/*  else if (button_hdl_pressed = true) {
    Serial.printf("key%d: repeating\n", num);
    button_hdl_presstime += TOUCH_MAXIMUM_TIME - TOUCH_MINIMUM_TIME;
    button_hdl_pressed = false;
  }
*/
  if (button_hdl_pressed) {
    return false;
  }
  if ((button_hdl_temp = (millis() - button_hdl_presstime)) < TOUCH_MINIMUM_TIME) {
    return false;
  }
  if (button_hdl_temp > TOUCH_MAXIMUM_TIME) {
    button_hdl_pressed = true;
    button_hdl_press = 0;
    return false;
  }
  button_hdl_pressed = true;
  button_hdl_press = 0;
  return true;
}

void button_hdl_up (void) {
  if (button_begin_proc(1)) {
    Serial.printf("button %d pressed\n", 1);
    if (clock_font == u8g2_font_8x13B_mr) {
      clock_font = u8g2_font_7x14B_mr;
    } else {
      clock_font = u8g2_font_8x13B_mr;
    }
  }
}

void button_hdl_center (void) {
  if (button_begin_proc(2)) {
    Serial.printf("button %d pressed\n", 2);
    displaying = 1;
  }
}

void button_hdl_down (void) {
  if (button_begin_proc(3)) {
    Serial.printf("button %d pressed\n", 3);
  }
}

void menu_draw (void) {
//  u8g2.setFont(u8g2_font_5x7_tf);
}

void setup_wifi(void) {
  WiFiManager wifiManager;
  WiFiManagerParameter custom_html1("<br/><center><b>Change NTP server</b></center>");
  WiFiManagerParameter custom_input1("ntp_server", "NTP server", "asia.pool.ntp.org", 100);
  WiFiManagerParameter custom_html2("<br/><center><b>Change timezone</b></center>");
  WiFiManagerParameter custom_input2("timezone", "timezone", "7", 100);
  wifiManager.setDebugOutput(false);
  wifiManager.addParameter(&custom_html1);
  wifiManager.addParameter(&custom_input1);
  wifiManager.addParameter(&custom_html2);
  wifiManager.addParameter(&custom_input2);

  if (!wifiManager.autoConnect("ESP32_hou_team01")) {
    Serial.println("Failed to connect, resetting...");
    if (WiFi.begin(WIFI_SSID, WIFI_PSK)) {
      int i = 0;
      while (WiFi.status() != WL_CONNECTED) {
        i++;
        if (i > 1000000) {
          break;
        }
      }
    }
    delay(1000);
    wifiManager.resetSettings();
    //ESP.eraseConfig();
    delay(1000);
    ESP.restart();
  }
  strcpy(ntp_server, custom_input2.getValue());
  sscanf(ntp_server, "%d", &timezone);
  if (timezone <= -12 || timezone >= 12) {
    timezone = 7;
  }
  strcpy(ntp_server, custom_input1.getValue());
  Serial.print("WiFI: ");
  Serial.println(WiFi.localIP());

}

void setup_clock(void) {
  
  bool rtc_found = false;
  Serial.print("DS3231 Status... ");
  if (!rtc.begin()) {
    Serial.println("NOT found");
  } else {
    rtc_found = true;
    Serial.println("OK");
  }

  if (!rtc_found) {
  } else {
    Serial.println("DS3231 time read test");
    uint32_t now1 = (uint32_t)rtc.now().unixtime();
    struct timeval now0;
    if (now1 < 1445000000) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Setting time with NTP srv %s", ntp_server);
        configTime(timezone * 3600, dst, ntp_server);
        Serial.print(".");
        struct tm timeinfo;
        while (!getLocalTime(&timeinfo)) {
          delay(100);
          Serial.print(".");
        }
        Serial.println(" OK");
        if (rtc_found) {
          rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
          Serial.println("DS3231 now initialized.");
      }
      } else {
        if (rtc_found) {
          rtc.adjust(DateTime(2010, 1, 1, 0, 0, 0));
          Serial.println("WiFi not connected, NTP first time syncing not available, defaults value applied.");
        } else {
          Serial.println("WiFi not connected, no time check available.");
        }
        //setTimeZone(GMT_OFFSET, GMT_DST);
      }
    }
    /* DateTime to struct timeval */
    now1 = (uint32_t)rtc.now().unixtime();
    now0 = { .tv_sec = now1 };
    settimeofday(&now0, NULL);
    Serial.print("DS3231 time read = ");
    Serial.print(now0.tv_sec);
    Serial.print(" (");
    Serial.print(now1);
    Serial.println(")");
    //setTimeZone(GMT_OFFSET, GMT_DST);
  }
}

void setup(void) {
  Serial.begin(BAUD);
  while (!Serial) {
    delay(5);
  }

  Serial.printf("ESP32 Serial = %d OK\n", BAUD);
  Serial.print("ESP32 SDK: ");
  Serial.println(ESP.getSdkVersion());
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));
  Serial.printf("%08X\n", (uint32_t)chipid);
  Serial.printf("CPU core %01d is main app0 processor\n", xPortGetCoreID());
  Serial.printf("CPU freq = %d mhz OK\n", 240);

  delay(1500);

  Wire.begin();
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  delay(100);

  setup_wifi();

  delay(100);

  setup_clock();

  delay(100);

  u8g2.begin();
  u8g2.setFontMode(0);
  u8g2.setContrast(255);

  delay(100);

  Serial.println("Starting activity = 0 (DisplayTime)");
  timer[0] = stimer.setInterval(1000, interval_DisplayTime);

  delay(100);

  setup_clock();

  delay(100);

  delay(100);

  Serial.print("Setup buttons using advanced GPIO interrupt handler...");

  touchAttachInterrupt(TOUCH_UP_PIN, button_hdl_up, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_CENTER_PIN, button_hdl_center, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_DOWN_PIN, button_hdl_down, TOUCH_THRESHOLD);

  Serial.println("Setup COMPLETED");
}

/* lấy tên ngày */
char* getWeekDayStr (int wday) {
  switch (wday) {
    case 0:
      return "Mon";
      break;
    case 1:
      return "Tue";
      break;
    case 2:
      return "Wed";
      break;
    case 3:
      return "Thu";
      break;
    case 4:
      return "Fri";
      break;
    case 5:
      return "Sta";
      break;
    case 6:
      return "Sun";
      break;
  }
}

void interval_DisplayTime (void) {
  if (displaying == 0) {
  static struct tm timeinfo;
  char timestr[2 + 1 + 2 + 1 + 2 + 1];
  static int old_yday = 0;
  static char old_datestr[3 + 2 + 2 + 1 + 2 + 1 + 4 + 1];
  u8g2.firstPage();
  if (!getLocalTime(&timeinfo)) {
    Serial.println("getLocalTime failed");
    return;
  }
  do {
    if (old_yday != timeinfo.tm_yday || old_yday == 0) {
      old_yday = timeinfo.tm_yday;
      snprintf(old_datestr, sizeof(old_datestr), "%s, %02d-%02d-%04d", getWeekDayStr(timeinfo.tm_wday), timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    }
    u8g2.setDrawColor(2);
    u8g2.setFont(clock_font);
    u8g2.drawStr(10, 20, old_datestr);
    snprintf(timestr, sizeof(timestr), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    u8g2.setDrawColor(1);
    u8g2.setFont(clock_font_small);
    u8g2.drawStr(0, 64, timestr);
  } while (u8g2.nextPage());
  } else if (displaying == 1) {
    
  }
}

void loop(void) {
  stimer.run();
}


