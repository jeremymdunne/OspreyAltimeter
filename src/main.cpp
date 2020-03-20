#include <Arduino.h>
#include <SensorPackage.h>
#include <RocketState.h>
#include <DataRecorder.h>
#include <DataFlags.h>
#include <FlightRecorder.h>

SensorPackage sensorPackage;
RocketState masterState;
DataRecorder dataRecorder;
FlightRecorder flightRecorder;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(2000);
  Serial.println("Hello!");
  int status = sensorPackage.begin(100000000000000,.1);
  if(status != 0){
    Serial.println("Sensor Package init fail: " + String(status));
    while(true);
  }
  Serial.println("Sensor Package init success");
  delay(100);
  status = dataRecorder.init();
  if(status != 0){
    Serial.println("Data Recorder init fail: " + String(status));
    while(true);
  }
  status = flightRecorder.init(&dataRecorder);
  if(status != 0){
    Serial.println("Data Recorder init fail: " + String(status));
    while(true);
  }
  Serial.println("Attempting Baro Read and Store!");
  //go get a baro reading
  delay(1000);
  int updateStatus = sensorPackage.update();
  sensorPackage.getData(&masterState.rawSensorData);
  Serial.println("Baro: " + String(masterState.rawSensorData.pressureData.pressure)); 
  int error = dataRecorder.recordStateData(RAW_BARO_TEMP_DATA_FLAG, &masterState);
  if(error < 0){
    Serial.println("\nData Recorder Error: " + String(error) + "; Local Error: " + String(dataRecorder.get_error()));
  }
  Serial.println("\nRecord Status: " + String(error));
  //Go and get some sizes
  Serial.println("Sensor Package Size: " + String(sizeof(sensorPackage)));
  Serial.println("Data Recorder Size: " + String(sizeof(dataRecorder)));
  Serial.println("Flight Recorder Size: " + String(sizeof(flightRecorder)));
  while(true);


}

ulong lastUpdate = 0;
int updateStatus = 0;
void loop() {
// put your main code here, to run repeatedly:
  updateStatus = sensorPackage.update();
  if(updateStatus > 0){
    Serial.println("\t" + String(micros() - lastUpdate));
    lastUpdate = micros();
  }

}
