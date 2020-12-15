#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

const int SOIL_SENSOR_DRYCEILING = 645;
const int SOIL_SENSOR_WETCEILING = 289;
const int WATERING_COOLDOWN = 5000;
const int IP = 1883;

int RELAY = 8;
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };  

unsigned long lastWatered = 0;

IPAddress ip(192, 168, 1, 102);
IPAddress server(192, 168, 1, 71); //Home IP address


EthernetClient ethClient;
PubSubClient client(ethClient);

void messageReceivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Check if the message was published on the topic wateringSystem
  if(strcmp(topic, "wateringSystem") == 0 && millis() - lastWatered >= WATERING_COOLDOWN){
    WaterPlant();
  }else{
    client.publish("wateringSystemFeedback", "[ {\"error\": { \"message\": \"Watering system is on cooldown!\" } } ]");
  }
}

void SubToChannels() {
  client.subscribe("wateringSystem");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Arduino")) {
      Serial.println("connected");
      SubToChannels();
    } else {
      Serial.print("\nConnection failed: rc=");
      Serial.print(client.state());
      Serial.println("try again in 3 seconds");
      delay(3000);
    }
  }
}

void setup()
{
  Serial.begin(57600);
  client.setServer(server, IP);
  client.setCallback(messageReceivedCallback);
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