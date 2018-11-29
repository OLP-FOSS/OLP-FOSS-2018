#include <WiFi.h>
#include <PubSubClient.h>
// is used in I2Cdev.h
#include <Wire.h>
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <I2Cdev.h>
#include <MPU6050.h>

// bạn thay đổi thông tin bên dưới cho phù hợp
const char* ssid =         "hoadx.phone";
const char* password =    "12345679";
const char* mqtt_server = "192.168.43.128";   /// lấy ip bằng lệnh ifconfig, ví dụ 192.168.1.105
const uint16_t mqtt_port = 1883;
const byte ledPin = 2;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
//float pitch,roll;

void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  
  pinMode(ledPin, OUTPUT);     // Khởi tạo LED
  Serial.begin(38400);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

}

void setup_wifi() {

  delay(10);
  // Kết nối wifi
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Bật LED nếu nhận được lệnh là 1
  if ((char)payload[0] == '1') {
     Serial.println("ON");
    digitalWrite(ledPin, LOW);   // LED có sẵn trên NodeMCU bật ở mức 0(LOW)
  } 
  // Tắt LED nếu nhận được lệnh là 0
 if ((char)payload[0] == '0') {
    Serial.println("OFF");
    digitalWrite(ledPin, HIGH);  // LED tắt ở mức 1(HIGH)
  }

}

void reconnect() {
  // Đợi tới khi kết nối
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Khi kết nối thành công sẽ gửi chuỗi helloworld lên topic event
      client.publish("event", "hello world");
      // ... sau đó sub lại thông tin
      client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void loop() {
  // read raw accel/gyro measurements from device
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
//  roll = atan(ax/sqrt(ay*ay+az*az));
//  pitch = atan(ay/sqrt(az*az+ax*ax));
//  roll = 0.0;
//  pitch = 0.0;
//  roll = atan2(ay, az)*180/PI;
//  pitch = atan2(ax, sqrt(ay*ay+az*az))*180/PI;
    
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  snprintf (msg, 75, "{\"ax\":\"%ld\",\"ay\":\"%ld\",\"az\":\"%ld\",\"gz\":\"%ld\"}", ax, ay, az, gz);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("event", msg);
  delay(1000);
}
