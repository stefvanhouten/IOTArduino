int ReadMoisture(){
    int sensorMeasurements = analogRead(A0);
    int percentageHumidity = map(sensorMeasurements, SOIL_SENSOR_WETCEILING, SOIL_SENSOR_DRYCEILING, 100, 0);
    // Serial.println((String)percentageHumidity + "% wet");
    return percentageHumidity;
}

void WaterPlant(){
    //Assign this value to the current time in ms
    lastWatered = millis();
    client.publish("wateringSystemFeedback", "[ {\"success\": { \"isWaterRunning\": \"true\" } } ]");
    digitalWrite(RELAY, HIGH);
    delay(1500);
    digitalWrite(RELAY, LOW);
    client.publish("wateringSystemFeedback", "[ {\"success\": { \"isWaterRunning\": \"false\" } } ]");
}

void HandlePlantCare(){
    int groundMoisture = ReadMoisture();
    //Check every 10 minutes
    if((millis() - lastWatered) >= 300000 && groundMoisture <= 40){
        WaterPlant();
    }
}