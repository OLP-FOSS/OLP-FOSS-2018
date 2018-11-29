/*
   HOU - VN
*/

/*
   Send to external OSCServer
   Pinout
*/

#define G_A 9.8
#define MPU_ACCEL_SCALE 4096
// 3.9 for g = 9.8m/s2
#define CONVERSION_G 3.9
#define MPU_WIFI_OSC

// math.h already in main source file
#include <driver/i2c.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <MPU6050_6Axis_MotionApps20.h>

#include <Arduino.h>

#include <math.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_deep_sleep.h>
#include <soc/rtc.h>

#include <SimpleTimer.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
//#include <U8g2lib.h>
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
//#include <freertos/FreeRTOS.h>
//#include <freertos/semphr.h>

#include <Wire.h>
#include <WiFi.h>

#include <U8g2lib.h>

#include <driver/gpio.h>
//#include <SPI.h>

#ifdef MPU_WIFI_OSC
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <OSCMessage.h>
#endif

// need right configuration
#define DEVICE_ID "team01"
#define WIFI_SSID "PTIT_LAB DAO TAO"
#define WIFI_PSK "buuchinh@"
#define MQTT_SERVER "10.170.46.231"
#define MQTT_PORT "1883"
#define WIFI_SERVER_IP IPAddress(10, 170, 46, 231)
#define MPU_INT_PIN 15
#define MPU_SDA_PIN 21
#define MPU_SCL_PIN 22
#define MPU_I2C_CLOCK 400000

//TwoWire Wire2(1);

#define BAUD 115200

MPU6050 mpu(MPU6050_ADDRESS_AD0_HIGH);
SimpleTimer stimer;
char mqtt_server[101];
int mqtt_port;
int timer[3] = { -1, -1, -1};
int timezone = +7;
WiFiClient wificlient;
PubSubClient client(wificlient);


struct configData {
  IPAddress mpu_udp_ip = IPAddress(127, 0, 0, 1);
  uint16_t mpu_udp_port = 8080;
};

struct configData config;

i2c_config_t i2cconf;
bool mpu_ready = false;
volatile bool mpu_interrupt = false;
uint16_t mpu_packet_size;
uint16_t mpu_fifo_count;
uint8_t mpu_int_status;
uint8_t mpu_fifo_buf[64];

uint16_t mpu_rate_count;

void IRAM_ATTR mpu_dmp_data_ready () {
  mpu_interrupt = true;
}


void mqtt_callback (char* topic, byte* payload, unsigned int length) {
  Serial.println(String(topic) + String(": ") + String((char*)payload));
}

void setup(void) {

  Serial.begin(BAUD);

  Serial.printf("ESP32 Serial = %d OK\n", BAUD);
  Serial.print("ESP32 SDK: ");
  Serial.println(ESP.getSdkVersion());
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID = %04X", (uint16_t)(chipid >> 32));
  Serial.printf("%08X\n", (uint32_t)chipid);
  Serial.printf("CPU core %01d is main app0 processor\n", xPortGetCoreID());
  Serial.printf("CPU freq = %d mhz OK\n", 240);

  while (!Serial) {
    delay(5);
  }

  WiFiManager wifiManager;
  WiFiManagerParameter custom_html1("<br/><center><b>Change MQTT server</b></center>");
  WiFiManagerParameter custom_input1("mqtt_server", "MQTT server", MQTT_SERVER, 100);
  WiFiManagerParameter custom_html2("<br/><center><b>Change MQTT port</b></center>");
  WiFiManagerParameter custom_input2("mqtt_port", "MQTT port", MQTT_PORT, 10);
  wifiManager.setDebugOutput(false);
  wifiManager.addParameter(&custom_html1);
  wifiManager.addParameter(&custom_input1);
  wifiManager.addParameter(&custom_html2);
  wifiManager.addParameter(&custom_input2);

  if (!wifiManager.autoConnect("ESP32_hou_team01")) {
    Serial.println("Failed to connect, resetting...");
    if (WiFi.begin(WIFI_SSID, WIFI_PSK)) {
      int i = 0;
      while (WiFi.status() != WL_CONNECTED) {
        i++;
        if (i > 1000000) {
          break;
        }
      }
    }
    delay(1000);
    wifiManager.resetSettings();
    //ESP.eraseConfig();
    delay(1000);
    ESP.restart();
  }

  strcpy(mqtt_server, custom_input2.getValue());
  sscanf(mqtt_server, "%d", &mqtt_port);
  if (mqtt_port <= 0 || mqtt_port >= 65535) {
    mqtt_port = 1883;
  }
  strcpy(mqtt_server, custom_input1.getValue());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  delay(100);

  Serial.print("WiFI: ");
  Serial.println(WiFi.localIP());

  /*
    u8g2.begin();
    u8g2.setFontMode(0);
    u8g2.setContrast(255);
  */

  delay(2500);

  config.mpu_udp_ip = WIFI_SERVER_IP;
  config.mpu_udp_port = 8080;

  // need driver i2c, clock mode 400kHz
  //Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN, MPU_I2C_CLOCK);
  //Wire2.setClock(MPU_I2C_CLOCK);

  i2cconf.mode = I2C_MODE_MASTER;
  i2cconf.sda_io_num = (gpio_num_t)MPU_SDA_PIN;
  i2cconf.scl_io_num = (gpio_num_t)MPU_SCL_PIN;
  i2cconf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  i2cconf.scl_pullup_en = GPIO_PULLUP_ENABLE;

  i2cconf.master.clk_speed = MPU_I2C_CLOCK;

  ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2cconf));

  ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

  mpu_rate_count = 0;

  Serial.print("MPU6050 initializing...");
  mpu.initialize();
  pinMode(MPU_INT_PIN, INPUT);

  // 1000/(1+99) should be 10Hz
  //mpu.setRate(99);
  // but it isn't
  mpu.setRate(9);
  // digital low pass filter
  mpu.setDLPFMode(MPU6050_DLPF_BW_98);

  int x = 0;

  if (mpu.testConnection()) {
    Serial.println(" OK");
    Serial.print("MPU6050 DMP6 initializing... ");

    if ((x = mpu.dmpInitialize()) == 0) {

      // 250 - 500 - 1000 - 2000
      mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
      // Default +-2G. Current +-8G, 4096/1G
      mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
      // -2120
      mpu.setXAccelOffset(-1252);
      // 758
      mpu.setYAccelOffset(-831);
      // -2488
      mpu.setZAccelOffset(1957);
      // 77
      mpu.setXGyroOffset(16);
      // -13
      mpu.setYGyroOffset(33);
      // -17
      mpu.setZGyroOffset(19);

      mpu.setDMPEnabled(true);

      mpu_int_status = mpu.getIntStatus();
      mpu_packet_size = mpu.dmpGetFIFOPacketSize();

      //digitalPinToInterrupt(MPU_INT_PIN);
      attachInterrupt(digitalPinToInterrupt(MPU_INT_PIN), mpu_dmp_data_ready, RISING);

      mpu_ready = true;

      Serial.println(" OK");

      mpu_interrupt = true;

    } else {
      Serial.print(" ecode=");
      Serial.print(x);
      Serial.println(" failed");
    }
  } else {
    Serial.println(" failed");
  }

  delay(100);

  while (!client.connected()) {
    Serial.print("MQTT connecting... ");
    if (client.connect("ESP32_hou_team01")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay(10000);
    }
  }

  Serial.println("Starting activity = 0 (MPURateCount)");
  timer[0] = stimer.setInterval(1000, interval_MPURateCount);

}

/*
  void loop_mpu_task (void* arg) {
  while(true) {
    vTaskDelay(5 / portTICK_PERIOD_MS);
    loop_mpu();
  }
  }
*/

void interval_MPURateCount (void) {
  if (mpu_rate_count != 0) {
    Serial.printf("mpu: rate = %d\n", mpu_rate_count);
    mpu_rate_count = 0;
  }
}

int calibration_size = 250;
int acel_deadzone = 8;
int giro_deadzone = 1;
int16_t ax, ay, az, gx, gy, gz;
int mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz, state = 0;
int ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset;

void meansensors(void) {
  long i = 0, buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;

  while (i < (calibration_size + 101)) {
    // read raw accel/gyro measurements from device
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (i > 100 && i <= (calibration_size + 100)) { //First 100 measures are discarded
      buff_ax = buff_ax + ax;
      buff_ay = buff_ay + ay;
      buff_az = buff_az + az;
      buff_gx = buff_gx + gx;
      buff_gy = buff_gy + gy;
      buff_gz = buff_gz + gz;
    }
    if (i == (calibration_size + 100)) {
      mean_ax = buff_ax / calibration_size;
      mean_ay = buff_ay / calibration_size;
      mean_az = buff_az / calibration_size;
      mean_gx = buff_gx / calibration_size;
      mean_gy = buff_gy / calibration_size;
      mean_gz = buff_gz / calibration_size;
    }
    i++;
    delay(2); //Needed so we don't get repeated measures
  }
}

void calibration(void) {
  ax_offset = -mean_ax / 8;
  ay_offset = -mean_ay / 8;
  az_offset = (MPU_ACCEL_SCALE - mean_az) / 8;

  gx_offset = -mean_gx / 4;
  gy_offset = -mean_gy / 4;
  gz_offset = -mean_gz / 4;
  while (1) {
    int ready = 0;
    mpu.setXAccelOffset(ax_offset);
    mpu.setYAccelOffset(ay_offset);
    mpu.setZAccelOffset(az_offset);

    mpu.setXGyroOffset(gx_offset);
    mpu.setYGyroOffset(gy_offset);
    mpu.setZGyroOffset(gz_offset);

    meansensors();
    Serial.println("...");

    if (abs(mean_ax) <= acel_deadzone) ready++;
    else ax_offset = ax_offset - mean_ax / acel_deadzone;

    if (abs(mean_ay) <= acel_deadzone) ready++;
    else ay_offset = ay_offset - mean_ay / acel_deadzone;

    if (abs(MPU_ACCEL_SCALE - mean_az) <= acel_deadzone) ready++;
    else az_offset = az_offset + (MPU_ACCEL_SCALE - mean_az) / acel_deadzone;

    if (abs(mean_gx) <= giro_deadzone) ready++;
    else gx_offset = gx_offset - mean_gx / (giro_deadzone + 1);

    if (abs(mean_gy) <= giro_deadzone) ready++;
    else gy_offset = gy_offset - mean_gy / (giro_deadzone + 1);

    if (abs(mean_gz) <= giro_deadzone) ready++;
    else gz_offset = gz_offset - mean_gz / (giro_deadzone + 1);

    if (ready == 6) break;
  }
}

void loop_calibration (void) {
  if (state == 0) {
    Serial.println("\nReading sensors for first time...");
    meansensors();
    state++;
    delay(1000);
  }

  if (state == 1) {
    Serial.println("\nCalculating offsets...");
    calibration();
    state++;
    delay(1000);
  }

  if (state == 2) {
    meansensors();
    Serial.println("\nFINISHED!");
    Serial.print("\nSensor readings with offsets:\t");
    Serial.print(mean_ax);
    Serial.print("\t");
    Serial.print(mean_ay);
    Serial.print("\t");
    Serial.print(mean_az);
    Serial.print("\t");
    Serial.print(mean_gx);
    Serial.print("\t");
    Serial.print(mean_gy);
    Serial.print("\t");
    Serial.println(mean_gz);
    Serial.print("Your offsets:\t");
    Serial.print(ax_offset);
    Serial.print("\t");
    Serial.print(ay_offset);
    Serial.print("\t");
    Serial.print(az_offset);
    Serial.print("\t");
    Serial.print(gx_offset);
    Serial.print("\t");
    Serial.print(gy_offset);
    Serial.print("\t");
    Serial.println(gz_offset);
    Serial.println("\nData is printed as: acelX acelY acelZ giroX giroY giroZ");
    Serial.println("Check that your sensor readings are close to 0 0 MPU_ACCEL_SCALE 0 0 0");
    Serial.println("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");
    while (1);
  }
}

void loop (void) {

  if (!mpu_ready) {
    return;
  }

/*
  loop_calibration();
  return;
*/

  if (!mpu_interrupt && mpu_fifo_count < mpu_packet_size) {
    return;
  }

  /*
    static uint8_t i;
    i = 0;
    // wait 1000 times for data
    while (!mpu_interrupt && mpu_fifo_count < mpu_packet_size) {
    i++;
    if (mpu_interrupt && mpu_fifo_count < mpu_packet_size) {
      mpu_fifo_count = mpu.getFIFOCount();
    } else if (i > 1000) {
      return;
    }
    }
  */

  // reset interrupt flag
  mpu_interrupt = false;
  mpu_int_status = mpu.getIntStatus();
  mpu_fifo_count = mpu.getFIFOCount();

  // check for overflow (inefficient cause this)
  if ((mpu_int_status & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || mpu_fifo_count >= 1024) {
    mpu.resetFIFO();
    mpu_fifo_count = mpu.getFIFOCount();
    Serial.println("MPU: FIFO overflow!");

    // check for DMP data ready interrupt (should happen frequently)
  } else if (mpu_int_status & _BV(MPU6050_INTERRUPT_DMP_INT_BIT)) {
    // wait for correct available data length, should be a VERY short wait
    while (mpu_fifo_count < mpu_packet_size) mpu_fifo_count = mpu.getFIFOCount();
    // read a packet from FIFO
    mpu.getFIFOBytes(mpu_fifo_buf, mpu_packet_size);
    // track FIFO count here in case there is > 1 packet available (immediately read more without waiting for an interrupt)
    mpu_fifo_count -= mpu_packet_size;

    // polling rate counter
    mpu_rate_count++;

    // quaternion q = x + y.i + z.j + w.k
    static Quaternion qat;
    // gravity (can be VectorFloat or uint16_t array)
    static VectorFloat grav;
    // acceleration x, y, z -> 3DOF
    static VectorInt16 accel_o;
    static VectorInt16 accel;
    //static VectorInt16 accel_w;
    // gyroscope yaw, pitch, roll -> +3DOF
    static float ypr[3];

    //mpu.getMotion6(&, &, &, &, &, &);
    mpu.dmpGetQuaternion(&qat, mpu_fifo_buf);
    mpu.dmpGetAccel(&accel, mpu_fifo_buf);
    mpu.dmpGetGravity(&grav, &qat);
    //mpu.dmpGetLinearAccel(&accel_o, &accel, &grav);
    // because of ta crappy function
    if (false) {
      accel_o.x = accel.x - (grav.x * (MPU_ACCEL_SCALE / 2));
      accel_o.y = accel.y - (grav.y * (MPU_ACCEL_SCALE / 2));
      accel_o.z = accel.z - (grav.z * (MPU_ACCEL_SCALE / 2));
    }
    //mpu.dmpGetLinearAccelInWorld(&accel_w, &accel_o, &qat);
    mpu.dmpGetYawPitchRoll(ypr, &qat, &grav);

    /*
        if (accel.z < -4000 && accel.z > -2100) {
          Serial.print("RIGHT Z ACCEL OFFSET = ");
          Serial.print(mpu.getZAccelOffset());
        } else {
          mpu.setZAccelOffset(mpu.getZAccelOffset() - 25);
        }
    */

#ifdef MPU_WIFI_OSC

    static WiFiUDP udp;
    static OSCMessage msg("/imu");

    if (WiFi.status() == WL_CONNECTED) {
      //Serial.printf("%f, %f, %f, %f\n", (float)qat.w, (float)qat.x, (float)qat.y, (float)qat.z);
      //Serial.printf("%d, %d, %d - (%f, %f, %f) , %f, %f, %f\n", accel.x, accel.y, accel.z, grav.x, grav.y, grav.z, ypr[0], ypr[1], ypr[2]);
      msg.add((int32_t)millis());
      msg.add((float)qat.w);
      msg.add((float)qat.x);
      msg.add((float)qat.y);
      msg.add((float)qat.z);
      msg.add(accel.x * (9.8 / (MPU_ACCEL_SCALE / 2)));
      msg.add(accel.y * (9.8 / (MPU_ACCEL_SCALE / 2)));
      msg.add(accel.z * (9.8 / (MPU_ACCEL_SCALE / 2)));
      msg.add(ypr[0]);
      msg.add(ypr[1]);
      msg.add(ypr[2]);
      udp.beginPacket(config.mpu_udp_ip, config.mpu_udp_port);
      msg.send(udp);
      udp.endPacket();
      msg.empty();

      if (client.connected()) {
        String str;
        Serial.println("ting...");
        str += "{\"device_id\":\"" + String(DEVICE_ID) +
               "\",\"ax\":" + (accel.x * (9.8 / (MPU_ACCEL_SCALE / 2))) + ",\"ay\":" + (accel.y * (9.8 / (MPU_ACCEL_SCALE / 2))) + ",\"az\":" + (accel.z * (9.8 / (MPU_ACCEL_SCALE / 2))) +
               ",\"gp\":" + ypr[0] + ",\"gy\":" + ypr[1] + ",\"gr\":" + ypr[2] + "}";
        client.publish("espx/mpu", str.c_str());
      }

    }

#endif

    /*
      #ifdef OUTPUT_READABLE_YAWPITCHROLL
        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, mpu_fifo_buf);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        Serial.print("ypr\t");
        Serial.print(ypr[0] * 180 / M_PI);
        Serial.print("\t");
        Serial.print(ypr[1] * 180 / M_PI);
        Serial.print("\t");
        Serial.println(ypr[2] * 180 / M_PI);
      #endif
      #ifdef OUTPUT_READABLE_REALACCEL
        // display real acceleration, adjusted to remove gravity
        mpu.dmpGetQuaternion(&q, mpu_fifo_buf);
        mpu.dmpGetAccel(&aa, mpu_fifo_buf);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        Serial.print("areal\t");
        Serial.print(aaReal.x);
        Serial.print("\t");
        Serial.print(aaReal.y);
        Serial.print("\t");
        Serial.println(aaReal.z);
      #endif
    */
  }

}


