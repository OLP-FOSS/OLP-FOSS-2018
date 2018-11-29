

#include "mqtt_connect.h"
#include "mpu.h"

PubSubClient myclient;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  // set up wifi
  setup_wifi();
  // set up screen
  //setup_display();
  // set up pubsub client 
  myclient = get_pubsub_client();
  // set up mpu
  setup_mpu();
  myclient.publish(topic_mpu_data, "I am cool");
}


void loop() {
  if(recording){
    String acel_ypr = get_acel_ypr_json();
    if(acel_ypr != ""){
      //Serial.println("acel_ypr");
      myclient.publish(topic_mpu_data, acel_ypr.c_str());
    }
    //Serial.println(acel_ypr.c_str());
//    myclient.publish(topic_mpu_data, acel_ypr.c_str());
//    Serial.println("Recording....");
//    Serial.println(get_acel_ypr_json().c_str());

  }
  //Serial.println(get_acel_ypr_json().c_str());
  //get_acel_ypr_json();
  //Serial.println(get_acelgyro_json().c_str());
  delay(10);
  myclient.loop();

}
