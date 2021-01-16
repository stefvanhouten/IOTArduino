#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 2 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

const int SOIL_SENSOR_DRYCEILING = 645;
const int SOIL_SENSOR_WETCEILING = 289;
const int WATERING_COOLDOWN = 5000;
const int IP = 1883;

const char* WATERING_SYSTEM = "WateringSystem";
const char* WATERING_SYSTEM_STATUS = "WateringSystem/Status";
const char* WATERING_SYSTEM_FEEDBACK = "WateringSystem/Feedback";
const char* COFFEE_SYSTEM_STATUS = "stat/coffee/RESULT";
const char* COFFEE_BROADCAST_STATUS = "Coffee/Status";
char* coffee_status = "OFF";

const char* PLANT_TEMPERATURE = "Plant/Temperature";
const char* PLANT_MOISTURE = "Plant/Moisture";
const char* PLANT_HUMIDITY = "Plant/Humidity";

bool watering_system_on = false;

int RELAY = 8;
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };  

unsigned long lastWatered = 0;
unsigned long lastMeasurementPublished = 0;

IPAddress ip(192, 168, 1, 102);
IPAddress server(192, 168, 1, 71); //Home IP address
EthernetClient ethClient;
PubSubClient client(ethClient);
DynamicJsonDocument doc(255);


void messageReceivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";
  for (int i=0;i<length;i++) {
    message.concat((char)payload[i]);
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Check if the message was published on the topic wateringSystem
  if(isSameTopic(topic, COFFEE_SYSTEM_STATUS)){
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    coffee_status= doc["POWER"];
    client.publish(COFFEE_BROADCAST_STATUS, coffee_status);
  }
  if(isSameTopic(topic, WATERING_SYSTEM) && isEligibleForWatering()){
    waterPlant();
  }else{
    client.publish(WATERING_SYSTEM_FEEDBACK, "Watering system is on cooldown");
  }
}

bool isSameTopic(char* firstValue, char* secondValue){
  return strcmp(firstValue, secondValue) == 0;
}

bool isEligibleForWatering(){
  return millis() - lastWatered >= WATERING_COOLDOWN;
}

void SubToChannels() {
  client.subscribe(WATERING_SYSTEM);
  client.subscribe(COFFEE_SYSTEM_STATUS);
}

int readMoisture(){
    int sensorMeasurements = analogRead(A0);
    int percentageHumidity = map(sensorMeasurements, SOIL_SENSOR_WETCEILING, SOIL_SENSOR_DRYCEILING, 100, 0);
    return percentageHumidity;
}

void waterPlant(){
    //Assign this value to the current time in ms
    lastWatered = millis();
    digitalWrite(RELAY, HIGH);
    watering_system_on = true;
    client.publish(WATERING_SYSTEM_STATUS, "ON");
    delay(750);
    digitalWrite(RELAY, LOW);
    watering_system_on = false;
    client.publish(WATERING_SYSTEM_STATUS, "OFF");
}

void publishSensorData(){
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if(!isnan(humidity) && !isnan(temperature)){
    if(watering_system_on){
      client.publish(WATERING_SYSTEM_STATUS, String("ON").c_str());
    }else{
      client.publish(WATERING_SYSTEM_STATUS, String("OFF").c_str());
    }
    client.publish(COFFEE_BROADCAST_STATUS, coffee_status);
    client.publish(PLANT_TEMPERATURE, String(temperature).c_str());
    client.publish(PLANT_HUMIDITY, String(humidity).c_str());
    client.publish(PLANT_MOISTURE, String(readMoisture()).c_str());
  }
  lastMeasurementPublished = millis();
}

void handlePlantCare(){
    int groundMoisture = readMoisture();
    if((millis() - lastMeasurementPublished) >= 1000){
      publishSensorData();
    }
    //Check every x minutes
    if((millis() - lastWatered) >= 300000 && groundMoisture <= 40){
        waterPlant();
    }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Arduino", "MobileClient", "1234")) {
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
  dht.begin();
  // Allow the hardware to sort itself out
  delay(2000);
}

void loop()
{
  handlePlantCare();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
