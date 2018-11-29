#include "mqtt_connect.h"

WiFiClient espClient;
PubSubClient client(espClient);

String topic_info;
String topic_gettime;
String topic_settime;
String topic_time;

bool recording = false;
bool screen_on = false;


void reconnect(){
  // wait till connection finising
  while(!client.connected()){
    Serial.print("Attempt to reconnect to MQTT server");
    if(client.connect("ESP32Client")){
      Serial.println("\nconnected");
      client.publish(topic_mpu_data, "ESP_reconnected");
      client.subscribe(topic_led_control);
      client.subscribe(topic_detect_gesture);
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5s
      delay(5000);
    }
  }
}

// Call back funtion when receiving signal from server
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char* msg = (char*)payload;
  char subbuff[length+1];
  memcpy(subbuff, &msg[0], length);
  subbuff[length] = '\0';
  //Serial.println("See watch: "+String(subbuff));
 
  if(String(topic)==topic_led_control){
    led_control(String(subbuff));
  }else if(String(topic)==topic_detect_gesture){
     Serial.print("gesture recognize");
    gesture_recognize(String(subbuff));
  }else if(String(topic)==topic_gettime){
    Serial.print("send time to server");
    //send_time_to_server();
  }else if(String(topic)==topic_settime){
    Serial.print("set time");
  }else if(String(topic) == topic_info){
    //display_username_password(String(subbuff));
  }
  Serial.println();
  
} 


PubSubClient get_pubsub_client(){
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    reconnect();

    // set topic that communicates with mobile app via mqtt
    topic_info = mac_add + "/info";
    topic_time = mac_add + "/time";
    topic_gettime = mac_add + "/request";
    topic_settime = mac_add + "/settime";

    // send mac address to server
    client.publish(topic_register, mac_add.c_str());

    // listen to signal from mobile app via mqtt
    client.subscribe(topic_gettime.c_str());
    client.subscribe(topic_settime.c_str());
    client.subscribe(topic_info.c_str());
    
    pinMode(ledPin, OUTPUT);
    return client;
}

void led_control(String msg){
    if (msg == "1"){
      // Kiểm tra nếu tin nhận được là 1 thì bật LED và ngược lại
      digitalWrite(ledPin, HIGH);
      recording = true;
    }if (msg == "0"){
      digitalWrite(ledPin, LOW);
      recording = false;
    }
}

void gesture_recognize(String msg){
  Serial.print(" label: "+msg+" ");
}

void send_time_to_server(){
  //client.publish(topic_time.c_str(), get_formatted_time().c_str());
}

void set_time_to_clock(){
  
}

void display_username_password(String msg){
  Serial.print("   uname + pass: " + msg);
}
