#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

boolean recording = true;
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); 
}

void loop() {
  if (recording) {
    if (irrecv.decode(&results)) {
      Serial.println("IR code recorded!");
      Serial.print(results.rawlen);
      Serial.println(" intervals.");
      recording = false;
    }
  } else {
    Serial.println("Sending  IR signal!");
    irsend.sendRaw((unsigned int*) results.rawbuf, results.rawlen, 38);
    delay(2000);
  }
}