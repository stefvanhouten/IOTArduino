/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

int LED = 8;
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };  
IPAddress ip(192, 168, 1, 102);
IPAddress server(192, 168, 1, 71);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  blink();
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Arduino")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("switches","hi");
      // ... and resubscribe
      client.subscribe("switches");
    } else {
      Serial.print("\nfailed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

void blink(){
  Serial.println(digitalRead(LED));
  Serial.println(digitalRead(LED) == LOW);
  Serial.println(digitalRead(LED) == HIGH);
  
  if(digitalRead(LED) == HIGH){
    digitalWrite(LED, LOW);
  }else{
    digitalWrite(LED, HIGH);
  }
}

void setup()
{
  Serial.begin(57600);

  pinMode(LED, OUTPUT);
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
