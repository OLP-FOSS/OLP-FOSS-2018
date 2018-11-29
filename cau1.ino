/* WiFi */
#include <WiFi.h>
char *ssid      = "PTIT_LAB DAO TAO";               // Ten WiFi SSID
char *password  = "buuchinh@";               // Mat khau wifi


/* Time Stamp */
#include <NTPClient.h>
#include <WiFiUdp.h>

#define NTP_OFFSET  7  * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "1.asia.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

/*font created at http://oleddisplay.squix.ch */
#include "modified_font.h"

/*for ssd1306*/
#include "SSD1306.h" 
#define SDA_PIN 21// GPIO21 -> SDA
#define SCL_PIN 22// GPIO22 -> SCL
#define SSD_ADDRESS 0x3c
SSD1306  display(SSD_ADDRESS, SDA_PIN, SCL_PIN);


void setup() 
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP32 OLED Display");
  Serial.println("");
  connectWiFi();
  timeClient.begin();

  display.init();

  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

}

void loop() 
{
  timeClient.update();
  displayData();
  delay(1000);
}

/*Display Data*/
void displayData() 
{
  String formattedTime = timeClient.getFormattedTime();
  Serial.print(formattedTime);

  display.clear();   /* clear display*/

  display.setFont(ArialMT_Plain_24);
  display.drawString(20, 31,  String(formattedTime));

  display.display();   /* write buffer to the display*/
  delay(10);
}


/* Connecting to a WiFi network*/

void connectWiFi(void)
{
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}
