#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <Wire.h>
#include <DS3231.h>

void setup_display();;
void setup_clock();
U8G2_SH1106_128X64_NONAME_1_HW_I2C get_display();
void showTime();
void showTem();
int touch();
String get_formatted_time();
