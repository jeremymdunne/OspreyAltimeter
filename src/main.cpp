#include <Arduino.h>
#include <SensorPackage.h>
#include <RocketState.h>
#include <DataRecorder.h>

SensorPackage sensorPackage;
RocketState masterState;
DataRecorder dataRecorder;

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
