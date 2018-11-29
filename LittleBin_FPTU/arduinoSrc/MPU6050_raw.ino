// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class
// 10/7/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added multiple output formats
//                 - added seamless Fastwire support
//      2011-10-07 - initial release


// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"
#include "SH1106Wire.h"
#include <Wire.h>
#include <DS3231.h>

// khai bao module DS3231
DS3231 clock;
RTCDateTime dt;
// tao mang luu ngay trong tuan
String dow[] = { "CN", "T2", "T3", "T4", "T5", "T6", "T7"};

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// oled 0.96 inch
SH1106Wire display(0x3C, 21, 22);

//MPU6050
const int MPU_addr=0x68;
int count = 0;
double yaw, pitch, roll;

#include <string.h>
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
//MPU6050 accelgyro;
MPU6050 accelgyro(0x68); // <-- use for AD0 high
int16_t ax, ay, az;
int16_t gx, gy, gz;
// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO
// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


int counting = 0;
int count1 = 0;
const int buttonPin = 2; 
int buttonState = 0; 
int buttonState2 = buttonState;
int menu_cvt = 0;
int menu_cvt2 = menu_cvt;


#define LED_PIN 13
bool blinkState = false;

void setup() {
    
    
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
    // it's really up to you depending on your project)
    Serial.begin(38400);

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    
    
    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // configure ESP32 LED pin for output
    pinMode(LED_PIN, OUTPUT);
    
    //run oled
    display.init();
    //display.setBrightness(brightness);
    //display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.drawString(10, 10, "FPT-UNIVERSITY");
    display.setFont(ArialMT_Plain_10);
    display.drawString(10,28, "LittleTeam");
    display.display();
    
    pinMode(buttonPin, INPUT);
    //run ds3231
    clock.begin();

  // Set sketch compiling time
    clock.setDateTime(__DATE__, __TIME__);
    delay(3500);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState2!=buttonState) {
    counting++;
    }
//  count1 %=3;
//
//  if(count1 ==2){
//    menu_cvt = 1;
//  }else{
//    menu_cvt = 0;
//  }
//  if (menu_cvt2!=menu_cvt) {
//    counting++;
//    }
  counting %= 3;


/////////////////////////////////////////////////////////   
  if (counting == 2) {
    //Menu
   accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print("a/g:\t");
        Serial.print(ax); Serial.print("\t");
        Serial.print(ay); Serial.print("\t");
        Serial.print(az); Serial.print("\t");
        Serial.print(gx); Serial.print("\t");
        Serial.print(gy); Serial.print("\t");
        Serial.println(gz);
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
    display.clear();
  
    yaw=gz/131.0;
    roll=atan2(ay,az);
    pitch=atan(ax/sqrt(ay*ay+az*az));
    //display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0,5, "AcX = " + String(ax) + " | AcY = " + String(ay));
    display.drawString(0,15,"AcZ = " + String(az) + " | Yaw = "+ String(yaw));
    display.drawString(0,25,"Roll = " + String(roll)+ " | Pitch = " + String(pitch));
    
   
    //display.drawString(0,38,);
    if (buttonState == HIGH) {
    menu_cvt=menu_cvt+1;
    } else menu_cvt == 0;
    delay(100);
    
    
    
    display.display();
    //delay(333);
    // read raw accel/gyro measurements from device
  } 
  ////////////////////////////////////////////////////////////
  if (counting == 1)  {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(47,0,"MENU" );
    display.drawString(20,10, "> Acel / gyro demo" );
    display.drawString(20,20," Infrared demo" );
    display.drawString(20,30," Heart-rate demo" );
    display.drawString(20,40," Setting" );
    display.drawString(20,50," About" );

    
    if (buttonState == HIGH) {
    menu_cvt=menu_cvt+1;
    } else menu_cvt == 2;
    display.display();
    delay(100);
    }
  //////////////////////////////////////////////////////////////////
    if (counting ==0) {
    display.clear();
    dt = clock.getDateTime();
    Serial.print(dt.year);   Serial.print("-");
    Serial.print(dt.month);  Serial.print("-");
    Serial.print(dt.day);    Serial.print(" ");
    Serial.print(dt.hour);   Serial.print(":");
    Serial.print(dt.minute); Serial.print(":");
    Serial.print(dt.second); Serial.println("");
    Serial.print(dt.dayOfWeek); Serial.println("");
    
    display.setFont(ArialMT_Plain_24);
    display.drawString(12,2,String(dt.hour)+"      "+String(dt.minute));
    
    display.setFont(ArialMT_Plain_16);
    display.drawString(57,0," = ");
     display.setFont(ArialMT_Plain_10);
    display.drawString(58,15,String(dt.second));

    display.setFont(ArialMT_Plain_16);
    display.drawString(12,28,String(dow[dt.dayOfWeek])+"  "+String(dt.day)+"/"+String(dt.month)+"/"+String(dt.year));
    display.display();
    
    delay(333);
    
   }
//end
buttonState2 = buttonState;
menu_cvt2 = menu_cvt;
}
