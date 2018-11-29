

#include "mqtt_connect.h"
#include "mpu.h"

PubSubClient myclient;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  // set up wifi
  setup_wifi();
  // set up pubsub client 
  myclient = get_pubsub_client();
  // set up mpu
  setup_mpu();
  myclient.publish(topic_mpu_data, "I am cool");
}


void send_data_to_server(){
  String acel_ypr = get_acel_ypr_json();
  if(acel_ypr != ""){
    // publish acel and yaw, pitch, roll json format
    myclient.publish(topic_mpu_data, acel_ypr.c_str());
  }
  
}

void loop() {
  if(recording){
    send_data_to_server();
  }

  delay(100);
  myclient.loop();

}
