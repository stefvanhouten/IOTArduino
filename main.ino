#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

int RELAY = 8;
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };  

IPAddress ip(192, 168, 1, 102);
IPAddress server(192, 168, 1, 71); //Home IP address

EthernetClient ethClient;
PubSubClient client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if(strcmp(topic, "wateringSystem") == 0){
    WaterPlant();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Arduino")) {
      Serial.println("connected");
      client.subscribe("switches");
      client.subscribe("wateringSystem");
    } else {
      Serial.print("\nfailed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait x seconds before retrying
      delay(3000);
    }
  }
}

void setup()
{
  Serial.begin(57600);
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  pinMode(RELAY, OUTPUT);

  // Allow the hardware to sort itself out
  delay(2000);
}

void loop()
{
  HandlePlantCare();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}