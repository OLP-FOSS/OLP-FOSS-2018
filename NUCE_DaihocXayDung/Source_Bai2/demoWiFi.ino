

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "I2Cdev.h"
#include "MPU6050.h"

#define SEND_PERIOD            1000
#define RECEIVE_PERIOD         5000

//const char* ssid = "PTIT_LAB_DAO_TAO";
//const char* password =  "buuchinh@";
const char* ssid = "KGM_TTCD";
const char* password =  "together@123";

char data_topic[] = "SM01";
char action_topic[] = "Power";
//char data_topic_TB[] = "v1/devices/me/telemetry";
char reply_topic[] = "device";
char device_id[] ="OLP_01";
uint16_t send_data_interval = SEND_PERIOD;
uint32_t lastTime;
uint32_t cycle_time;
uint16_t reconnect_time;

int16_t ax,ay,az,gx,gy,gz;

WiFiClient espClient;
PubSubClient client(espClient);

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

#define OUTPUT_READABLE_ACCELGYRO

void setup() {
//  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

 client.setServer("iot.nuce.space", 1883);
 client.setCallback(callback);
 
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

}

void prepare_send_data (char * buf, unsigned int length)
{
  
     sprintf(buf,"{\"0\":%d,\"1\":%d,\"2\":%d,\"3\":%d,\"4\":%d,\"5\":%d}",
               ax,ay,az,gx,gy,gz);
//     sprintf(buf,"%d,%d,%d,%d,%d,%d",
//               ax,ay,az,gx,gy,gz); 
//       sprintf(buf,"%d",ax);
}


 
void loop() {
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

  
  if (!client.connected()) {
    reconnect();
    reconnect_time++;
  }
  
  if(millis() - lastTime > send_data_interval)
  { 
      char msg[96];
      prepare_send_data(msg, 96); 
      client.publish(data_topic,msg);
      lastTime = millis();
      delay(1000L);
  }
  delay(4000L);
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  
  while (!client.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    char user[] = "at_ktht_03";    //"wiqusarz";
    char pass[] = "ktht@2018";    //"rj3naMiQKof4";
    char login_msg[]="OK";
    char connection_ok[] = "OK! RabbitMQ MQTT Server connected";
    char connection_fail[] = "Failed, rc=";
    if (client.connect(device_id,user,pass)) {
      Serial.println(connection_ok);
      // Once connected, publish an announcement...
      client.publish(reply_topic,login_msg);
      // ... and resubscribe
      client.subscribe(action_topic);
    } else {
       Serial.print(connection_fail);
       Serial.print(client.state());
       Serial.println("Wait");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }  
}

//print any message received for subscribed topic
void callback(char* action_topic, byte* payload, unsigned int length) {
  Serial.print("[");
  Serial.print(action_topic);
  Serial.print("]");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  
}

