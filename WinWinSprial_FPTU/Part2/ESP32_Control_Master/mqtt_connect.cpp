#include "mqtt_connect.h"

WiFiClient espClient;
PubSubClient client(espClient);

String topic_info;
String topic_gettime;
String topic_settime;
String topic_time;

bool recording = true;
bool screen_on = false;


void reconnect(){
  // wait till connection finising
  while(!client.connected()){
    Serial.print("Attempt to reconnect to MQTT server");
    if(client.connect("ESP32Client")){
      Serial.println("\nconnected");
      client.publish(topic_mpu_data, "ESP_reconnected");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5s
      delay(5000);
    }
  }
}



PubSubClient get_pubsub_client(){
    client.setServer(mqtt_server, mqtt_port);
    reconnect();
    pinMode(ledPin, OUTPUT);
    return client;
}
