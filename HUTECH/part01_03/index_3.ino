/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

const int MPU_addr=0x68; // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;


#include "RTClib.h"       //to show time

#include <IRremote.h>
int RECV_PIN = 13;
IRsend irsend;
IRrecv irrecv(RECV_PIN);

decode_results results;

//  Touch Sensor Pin Layout
//   T0 = GPIO4
//   T1 = GPIO0
//   T2 = GPIO2
//   T3 = GPIO15
//   T4 = GPIO13
//   T5 = GPIO12
//   T6 = GPIO14
//   T7 = GPIO27
//   T8 = GPIO33
//   T9 = GPIO32 */

int mode = 0; //0: clock_mode; 1: menu_mode
int currentChosingMenuItem = 0; 
RTC_DS3231 rtc;

// Include custom images
//#include "images.h"



char daysOfTheWeek[7][12] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};


// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, 21, 22);
// SH1106 display(0x3c, D3, D5);






int buff(int pin)                                       //Function to handle the touch raw sensor data
{

  int out = (50 - touchRead(pin));                         //  Scale by n, value very sensitive currently
  // change to adjust sensitivity as required
  if (out > 0 )
  {
    return (out + 2);
  }
  else
  {
    return 0;                                        //Else, return 0
  }

}


void setup() {

  
Wire.begin();
Wire.beginTransmission(MPU_addr);
Wire.write(0x6B); // PWR_MGMT_1 register
Wire.write(0); // set to zero (wakes up the MPU-6050)
Wire.endTransmission(true);


  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);


  
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  
if (! rtc.begin()) {
Serial.println("Couldn't find RTC");
while (1);
}

rtc.adjust(DateTime(__DATE__, __TIME__));
  delay(1000);
}

void drawAbout() {
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, String("--------- ABOUT ---------"));
    
    display.drawString(64, 13, String("This is developed by"));
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 28, String("HUTECH"));
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 53, String("Olympic FOSS 2018"));    

//    dedicated and passionate in working with new technologies. Hutech - 2018 - FOSS
}

void drawMenu() {
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, String("--------- MENU ---------"));
    
    display.drawString(64, 13, String("Accel / gyro demo"));
    display.drawString(64, 23, String("Infrared demo"));
    display.drawString(64, 33, String("Heart-rate demo"));
    display.drawString(64, 43, String("Setting"));
    display.drawString(64, 53, String("About"));

    display.drawRect(19, 15 + currentChosingMenuItem * 10, 90, 10);
}

void drawClock() {
DateTime now = rtc.now();
  
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_24);
    display.drawString(40, 10, String(now.hour()));
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 8, ":");
    display.setFont(ArialMT_Plain_24);
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(88, 10, String(now.minute()));
    display.setFont(ArialMT_Plain_16);
    
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 25, String(now.second()));
  
    display.setFont(ArialMT_Plain_16);
  
    char strne[10]; 
    int i=now.day(); 
    itoa(i, strne, 10);
    char str[255];
    strcpy(str, daysOfTheWeek[now.dayOfTheWeek()]);
    strcat(str, ", ");
    if(sizeof(strne) == 1) 
      strcat(str, "0");
    strcat(str, strne);
    strcat(str, "/");
    char strne2[10]; 
    int i2=now.month(); 
    itoa(i2, strne2, 10);
    if(sizeof(strne2) == 1) 
      strcat(str, "0");
     strcat(str, strne2);
    
    strcat(str, "/");
    char strne3[10]; 
    int i3=now.year(); 
    itoa(i3, strne3, 10);
    strcat(str, strne3);
    display.drawString(64, 44, String(str));
}

void loop() {
  display.clear();


    
   int menuToggle = buff(T7);
   if(menuToggle > 0) {
      mode++;
      mode = mode % 2;
   }
   
   int menuPositionToggle = buff(T5);
   if(menuPositionToggle > 0) {
      currentChosingMenuItem++;
      currentChosingMenuItem = currentChosingMenuItem%5;
   } 

   
  if(mode == 0)
    drawClock();
  else if(mode == 1) {
    drawMenu();

    
     int toggleFeature = buff(T6);
     if(toggleFeature > 0) {
        if(toggleFeature) {


            if(currentChosingMenuItem == 1) { //remote
                Serial.println("toggle remote demo");
                for (int i = 0; i < 3; i++) {
                  irsend.sendSAMSUNG(0xE0E040BF, 12);
                  delay(40);
                }
            } else if(currentChosingMenuItem == 0) {
                Serial.println("toggle accel demo");
            }
            else if(currentChosingMenuItem == 4) {
                if(mode == 4) { 
                  mode = 1;
                } else {
                  mode = 4;
                }
            }
        }
     } 
  } else {
    drawAbout();
  }
  
 

  display.display();


//  if (irrecv.decode(&results)) {
//    Serial.println(results.value, HEX);
//    irrecv.resume(); // Receive the next value
//  }



//                  irsend.sendSAMSUNG(0xE0E040BF, 12);
//                  irsend.sendSony(0xE0E040BF, 12);




//
//Wire.beginTransmission(MPU_addr);
//Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
//Wire.endTransmission(false);
//Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers
//AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
//AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
//AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
//Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
//GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
//GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
//GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
//Serial.print("AcX = "); Serial.print(AcX);
//Serial.print(" | AcY = "); Serial.print(AcY);
//Serial.print(" | AcZ = "); Serial.print(AcZ);
//Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53); //equation for temperature in degrees C from datasheet
//Serial.print(" | GyX = "); Serial.print(GyX);
//Serial.print(" | GyY = "); Serial.print(GyY);
//Serial.print(" | GyZ = "); Serial.println(GyZ);


if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
   
   delay(100);
}
