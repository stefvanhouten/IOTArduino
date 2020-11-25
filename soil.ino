const int DRYCEILING = 645;
const int WETCEILING = 289;

void ReadMoisture(){
    int sensorMeasurements = analogRead(A0);
    int percentageHumidity = map(sensorMeasurements, WETCEILING, DRYCEILING, 100, 0);
    Serial.println((String)percentageHumidity + "% wet");
    delay(1000);
}