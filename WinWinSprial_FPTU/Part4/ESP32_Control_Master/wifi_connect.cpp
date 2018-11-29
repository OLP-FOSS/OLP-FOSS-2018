#include "wifi_connect.h"

String mac_add;

void setup_wifi(){
  delay(10);
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

  // set mac address
  mac_add = WiFi.macAddress();
  Serial.println("Mac address: "+String(mac_add));
}
