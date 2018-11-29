#include "display.h"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

int16_t pinTouch[] = {21, 22, 23, 24, 20, 18, 17, 16, 13, 12};

int pre = -1;

int count = 0;
int last = 1;
unsigned long time_now = 0;
unsigned long time_now_tem = 0;
#define LED_PIN 13
bool blinkState = false;

DS3231 my_clock;
RTCDateTime dt;


void setup_display(void) {
  u8g2.begin();
  u8g2.enableUTF8Print();

  // Initialize DS3231
  Serial.println("Initialize DS3231");;
  
  // set up clock 
  setup_clock();
}

void setup_clock(){
    my_clock.begin();
    my_clock.setDateTime(__DATE__, __TIME__);
}

U8G2_SH1106_128X64_NONAME_1_HW_I2C get_display(){
    return u8g2;
}


void showTime() {

  dt = my_clock.getDateTime();
  if (millis() > (time_now + 500) ) {
    time_now = millis();
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

String get_formatted_time(){
  dt = my_clock.getDateTime();
  return String(dt.hour)+":"+String(dt.minute);
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
     showTem();

  }
  if (decision < 0) {
    Serial.println("left");
        showTime();

  }
  Serial.println(decision);

}

void showTem()
{
  if (millis() > (time_now_tem + 1000) ) {
    time_now_tem = millis();
    my_clock.forceConversion();
    u8g2.firstPage();
//    Serial.print("Temperature: ");
//    Serial.println(my_clock.readTemperature());
    do {
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(0, 48, 69);
      u8g2.setFont(u8g2_font_logisoso32_tf);
      u8g2.setCursor(48 + 3, 42);
      u8g2.print(int(my_clock.readTemperature()));
      u8g2.print("°C");
    } while ( u8g2.nextPage() );
  }
}
