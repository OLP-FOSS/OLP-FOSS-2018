
// CONNECTIONS:
// DS1307 SDA --> SDA
// DS1307 SCL --> SCL
// DS1307 VCC --> 5v
// DS1307 GND --> GND

/* for software wire use below
#include <SoftwareWire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>

SoftwareWire myWire(SDA, SCL);
RtcDS1307<SoftwareWire> Rtc(myWire);
 for software wire use above */

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);
#include <GOFi2cOLED.h>
#include <IRremote.h>

byte IR_LED = 12;
IRsend irsend(IR_LED);

int RECV_PIN = 15;
IRrecv irrecv(RECV_PIN);
decode_results results;


GOFi2cOLED GOFoled;
unsigned int day;
unsigned int month;;
unsigned int _year;
unsigned int second;
unsigned int minute;
unsigned int _hour;
int threshold = 18;
bool touch1detected1 = false;

void gotTouch1(){
 touch1detected1 = true;
}
bool touch1detected2 = false;

void gotTouch2(){
 touch1detected2 = true;
}
/* for normal hardware wire use above */

void setup () 
{
    Serial.begin(57600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Cuases:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");

        // following line sets the RTC to the date & time this sketch was compiled
        // it will also reset the valid flag internally unless the Rtc device is
        // having an issue

        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low); 
        Serial.println();
      GOFoled.init(0x3C);
  GOFoled.display(); // show splashscreen
  delay(2000);
  GOFoled.clearDisplay();

  GOFoled.setTextSize(1);
  GOFoled.setTextColor(WHITE);
    irrecv.enableIRIn(); // Start the receiver
}
#define POWER         0xE0E040BF            //from IRrecvDemo, Ken Shirriff
#define SAMSUNG_BITS  32
void loop () 
{   
   Serial.println(touchRead(T0));
  if(touchRead(T0)<30){
       GOFoled.setCursor(10, 56);
      GOFoled.print("Nhan ma hong ngoai");
       GOFoled.display();
        GOFoled.clearDisplay();
    getIr();
  }
    if(touchRead(T1)<30){
     irsend.sendSAMSUNG(POWER, SAMSUNG_BITS); 
  }
    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Cuases:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

RtcDateTime currTime = Rtc.GetDateTime();
  printDateTime(currTime);
  //  delay(10000);
  day    = currTime.Day();
  month  = currTime.Month();
  _year  = currTime.Year();
  second = currTime.Second();
  minute = currTime.Minute();
  _hour  = currTime.Hour();
  GOFoled.setCursor(35, 20);
//    GOFoled.print("www.arduino.vn");
//    GOFoled.print(timeClient.getFormattedTime());
  GOFoled.print(month); GOFoled.print("/"); GOFoled.print(day); GOFoled.print("/"); GOFoled.print(_year);
  GOFoled.print(" ");
  GOFoled.setCursor(40, 38);
  GOFoled.print(_hour); GOFoled.print(":"); GOFoled.print(minute); GOFoled.print(":"); GOFoled.print(second);
  GOFoled.display();
  delay(10);
  GOFoled.clearDisplay();
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

void getIr(){
  int stopp = 0;
  while(!stopp){
    if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    Serial.println("abc");
    irrecv.resume(); // Receive the next value
    stopp = 1;
    delay(100);
   }
  }
}
