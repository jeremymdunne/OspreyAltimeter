#include <SensorPackage.h>


int SensorPackage::getData(RawSensorData *data){
  //copy the data over, don't give access to local copy
  copyRawSensorData(&rawData,data);
  return 0;
}

int SensorPackage::update(){
  int updates = 0;
  if(millis() - icmData.timestamp/1000 > imuUpdateMillis){
    updateImu();
    //Serial.println("Update IMU");
    updates ++;
  }
  if(millis() - rawData.pressureData.timestamp/1000 > bmpUpdateMillis){
    updateBmp();
    Serial.println("Update BMP");
    updates ++;
  }
  //return the number of available updates
  return updates;
}

int SensorPackage::begin(float imuUpdateRate, float pressureUpdateRate){
  int status = initImu(imuUpdateRate);
  if(status != 0) return status;
  status = initBmp(pressureUpdateRate);
  if(status != 0) return status;
  //go and get some data!
  updateImu();
  updateBmp();
  return 0;
}

int SensorPackage::initImu(int rate){
  imuUpdateMillis = (int)(1000.0/rate + .5);
  mostRecentErrorCode = icm.init(ICM20948_ADDR_AD0_HIGH, ICM20948::ACCEL_16_GRAVITIES, ICM20948::GYRO_2000_DPS, ICM20948::ACCEL_DPLF_50_HZ, ICM20948::GYRO_DLPF_73_HZ, true);
  if(mostRecentErrorCode != 0){
    imuFailure = true;
    return IMU_INIT_FAILURE;
  }
  return 0;
}

int SensorPackage::initBmp(int rate){
  bmpUpdateMillis = (int)(1000.0/rate + .5);
  mostRecentErrorCode = bmp.begin();
  if(mostRecentErrorCode != 0){
    bmpFailure = true;
    return BMP_INIT_FAILURE;
  }
  bmp.setFilter(BMP280::FILTER_8);
  return 0;
}

void SensorPackage::setPressureUpdateRate(int rate){
  bmpUpdateMillis = (int)(1000.0/rate + .5);
}

void SensorPackage::setImuUpdateRate(int rate){
  imuUpdateMillis = (int)(1000.0/rate + .5);
}

int SensorPackage::updateImu(){
  icm.getData(&icmData);
  copyImuData(&icmData, &rawData.imuData);
  return 0;
}

int SensorPackage::updateBmp(){
  bmp.readSensor(&rawData.pressureData.temperature, &rawData.pressureData.pressure);
  rawData.pressureData.timestamp = micros();
  return 0;
}

void SensorPackage::copyImuData(ICM20948::RawData *source, RawImuData *target){
  for(int i = 0; i < 3; i ++){
    target->accel[i] = source->accel[i];
    target->gyro[i] = source->gyro[i];
    target->mag[i] = source->mag[i];
  }
  target->temperature = source->temperature;
  target->timestamp = source->timestamp;
}
