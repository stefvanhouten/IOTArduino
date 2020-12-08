const int DRYCEILING = 645;
const int WETCEILING = 289;
unsigned long lastWatered = 0;

int ReadMoisture(){
    int sensorMeasurements = analogRead(A0);
    int percentageHumidity = map(sensorMeasurements, WETCEILING, DRYCEILING, 100, 0);
    // Serial.println((String)percentageHumidity + "% wet");
    return percentageHumidity;
}

void WaterPlant(){
    //Assign this value to the current time in ms
    lastWatered = millis();
    Serial.println("WATERING PLANT!");
    digitalWrite(RELAY, HIGH);
    delay(4000);
    digitalWrite(RELAY, LOW);
}

void HandlePlantCare(){
    int groundWettness = ReadMoisture();
    //Check every 10 minutes
    if((millis() - lastWatered) >= 30000 && groundWettness <= 40){
        WaterPlant();
    }
}