#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <time.h>

#define SEND_PIN 5
#define RECV_PIN 18

#include <IRremote.h>

IRsend irsend(SEND_PIN);
IRrecv irrecv(RECV_PIN);

#define POWER_CODE 0xE0E040BF
#define BITS_CODE 32

#define TOUCH_UP_PIN 14
#define TOUCH_CENTER_PIN 12
#define TOUCH_DOWN_PIN 13
// Qua không khí thì touch threshold thường là 100-120, khi chạm gần là 5-15, kết nối trực tiếp là 1
#define TOUCH_THRESHOLD 16
// tối đa 1200ms chỉ nhận 1 lần touch
#define TOUCH_MAXIMUM_TIME 1200l
// chạm tối thiếu 150ms để nhận touch
#define TOUCH_MINIMUM_TIME 250l

#define BAUD 115200

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
  }
}

void button_hdl_center (void) {
  if (button_begin_proc(2)) {
    Serial.printf("button %d pressed\n", 2);
    irsend.sendSAMSUNG(POWER_CODE, BITS_CODE);
    delay(50);
    irsend.sendSAMSUNG(POWER_CODE, BITS_CODE);
    delay(100);
  }
}

void button_hdl_down (void) {
  if (button_begin_proc(3)) {
    Serial.printf("button %d pressed\n", 3);
  }
}

void setup() {
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

  irrecv.enableIRIn();

  delay(100);

  delay(100);

  Serial.print("Setup buttons using advanced GPIO interrupt handler...");

  touchAttachInterrupt(TOUCH_UP_PIN, button_hdl_up, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_CENTER_PIN, button_hdl_center, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_DOWN_PIN, button_hdl_down, TOUCH_THRESHOLD);

  Serial.println("Setup COMPLETED");
}

void loop() {
  delay(5);
}


