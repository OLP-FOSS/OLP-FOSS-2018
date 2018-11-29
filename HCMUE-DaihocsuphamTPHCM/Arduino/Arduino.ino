#include <Arduino.h>
#include <U8g2lib.h>
#include <string.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#define ssid "PTIT_LAB DAO TAO"
#define password "buuchinh@"

#define mqtt_server "10.170.46.243" 
#define mqtt_topic_sub "demo"

const uint16_t mqtt_port = 1883; 

WiFiClient espClient;
PubSubClient client(espClient);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
MPU6050 mpu;
String s;
/* Địa chỉ của DS1307 */
const byte DS1307 = 0x68;
/* Số byte dữ liệu sẽ đọc từ DS1307 */
const byte NumberOfFields = 7;
 
/* khai báo các biến thời gian */
int second, minute, hour, day, wday, month, year;
char buf[9];
Vector normAccel;
  int pitch ;
  int roll ;
  int yaw;
char msgAx[50],msgAy[50],msgAz[50],msgYaw[50],msgPitch[50],msgRoll[50],msgTime[50];
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void inMainPage(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_inr21_mn); // Font 22 pixel số
    if (hour >=10) 
    snprintf (buf, 9, "%d", hour);
    else snprintf (buf, 9, "0%d", hour);
    u8g2.drawStr(20,40,buf);  // write something to the internal memory
    if (minute >=10) 
    snprintf (buf, 9, "%d", minute);
    else snprintf (buf, 9, "0%d", minute);
    u8g2.drawStr(70,40,buf);
    u8g2.drawStr(55,40,":");
    u8g2.setFont(u8g2_font_timR10_tf); // Font 10
    if (month >=10) 
    snprintf (buf, 9, "Th%d", month);
    else snprintf (buf, 9, "Th0%d", month);
    u8g2.drawStr(0,10,buf);
    if (day >=10)
    snprintf (buf, 9, "%d", day);
    else snprintf (buf, 9, "0%d", day);
    u8g2.drawStr(35,10,buf);
    
    snprintf (buf, 9, "%d", year);
    u8g2.drawStr(55,10,buf);
    
    u8g2.drawLine(0,12,128,12);
    u8g2.setFont(u8g2_font_timR10_tf); // Font 10

    if (second >=10)
    snprintf (buf, 9, "%d", second);
    else snprintf (buf, 9, "0%d", second);
    u8g2.drawStr(110,40,buf);

    snprintf(buf,9,"X : %d",(int)normAccel.XAxis);
    u8g2.drawStr(0,60,buf);
    snprintf(buf,9,"Y : %d",(int)normAccel.YAxis);
    u8g2.drawStr(50,60,buf);
    snprintf(buf,9,"Z : %d",(int)normAccel.ZAxis);
    u8g2.drawStr(90,60,buf);
    
    
  } while (u8g2.nextPage());
  // transfer internal memory to the display
  u8g2.sendBuffer();          // transfer internal memory to the display
}
void readDS1307()
{
        Wire.beginTransmission(DS1307);
        Wire.write((byte)0x00);
        Wire.endTransmission();
        Wire.requestFrom(DS1307, NumberOfFields);
        
        second = bcd2dec(Wire.read() & 0x7f);
        minute = bcd2dec(Wire.read() );
        hour   = bcd2dec(Wire.read() & 0x3f); // chế độ 24h.
        wday   = bcd2dec(Wire.read() );
        day    = bcd2dec(Wire.read() );
        month  = bcd2dec(Wire.read() );
        year   = bcd2dec(Wire.read() );
        year += 2000;    
}
/* Chuyển từ format BCD (Binary-Coded Decimal) sang Decimal */
int bcd2dec(byte num)
{
        return ((num/16 * 10) + (num % 16));
}
/* Chuyển từ Decimal sang BCD */
int dec2bcd(byte num)
{
        return ((num/10 * 16) + (num % 10));
}
 
void digitalClockDisplay(){
    // digital clock display of the time
    Serial.print(hour);
    printDigits(minute);
    printDigits(second);
    Serial.print(" ");
    Serial.print(day);
    Serial.print(" ");
    Serial.print(month);
    Serial.print(" ");
    Serial.print(year); 
    Serial.println(); 
}
 
void printDigits(int digits){
    // các thành phần thời gian được ngăn chách bằng dấu :
    Serial.print(":");
        
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}
 
/* cài đặt thời gian cho DS1307 */
void setTime(byte hr, byte minu, byte sec, byte wd, byte d, byte mth, byte yr)
{
        Wire.beginTransmission(DS1307);
        Wire.write(byte(0x00)); // đặt lại pointer
        Wire.write(dec2bcd(sec));
        Wire.write(dec2bcd(minu));
        Wire.write(dec2bcd(hr));
        Wire.write(dec2bcd(wd)); // day of week: Sunday = 1, Saturday = 7
        Wire.write(dec2bcd(d)); 
        Wire.write(dec2bcd(mth));
        Wire.write(dec2bcd(yr));
        Wire.endTransmission();
}
void callback(char* topic, byte* payload, unsigned int length) {
  s="";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    s+=(char)payload[i];
  }
  
  Serial.println();
  //xuLyChuoi(s);
  Serial.print(s);


  delay(1000);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.publish("ax", "ESP_reconnected");
      client.publish("ay", "ESP_reconnected");
      client.publish("az", "ESP_reconnected");
      client.publish("pitch", "ESP_reconnected");
      client.publish("roll", "ESP_reconnected");
      client.publish("yaw", "ESP_reconnected");
      client.subscribe("Server");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void setup() {
  
  u8g2.begin();
  Wire.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port); 
  client.setCallback(callback);
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_unifont_t_vietnamese2); // choose a suitable font
  /* cài đặt thời gian cho module */
  setTime(15, 23, 45, 5, 29, 11, 18); // 12:30:45 CN 08-02-2015

  Serial.begin(115200);
}
void loop() {
  /* Đọc dữ liệu của DS1307 */
 //u8g2.clearBuffer();  
  readDS1307();
  
  digitalClockDisplay();
  
  normAccel = mpu.readNormalizeAccel();
  pitch = (atan(normAccel.XAxis/sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  roll = (atan(normAccel.YAxis/sqrt(normAccel.XAxis*normAccel.XAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  yaw = (atan(normAccel.ZAxis/sqrt(normAccel.XAxis*normAccel.XAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  inMainPage();
  Serial.print(" Xnorm = ");  Serial.print(normAccel.XAxis);
  Serial.print(" Ynorm = ");  Serial.print(normAccel.YAxis);
  Serial.print(" Znorm = ");  Serial.print(normAccel.ZAxis);  Serial.println();
  
//  snprintf(buf,9,"X:%d",(int)normAccel.XAxis);
//    u8g2.drawStr(0,60,buf);
//    snprintf(buf,9,"Y:%d",(int)normAccel.YAxis);
//    u8g2.drawStr(40,60,buf);
//    snprintf(buf,9,"Z:%d",(int)normAccel.ZAxis);
//    u8g2.drawStr(80,60,buf);
//    u8g2.sendBuffer();
//
//  Serial.print(" Pitch = ");  Serial.print(pitch);
//  Serial.print(" Roll = ");   Serial.print(roll);
//  Serial.print(" Yaw = ");    Serial.print(yaw);              Serial.println();
//
//  float temp = mpu.readTemperature();
//  Serial.print(" Temp = ");   Serial.print(temp);             Serial.println(" *C");  


if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
    snprintf (msgAx, 75, "%ld", normAccel.XAxis); client.publish("ax", msgAx);
    snprintf (msgAy, 75, "%ld", normAccel.YAxis); client.publish("ay", msgAy);
    snprintf (msgAz, 75, "%ld", normAccel.ZAxis); client.publish("az", msgAz);

    snprintf (msgYaw, 75, "%ld", yaw);            client.publish("yaw", msgYaw);
    snprintf (msgPitch, 75, "%ld", pitch);        client.publish("pitch", msgPitch);
    snprintf (msgRoll, 75, "%ld", roll);          client.publish("roll", msgRoll);

  delay(100);  
}
