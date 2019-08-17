/*
The sensor package handles everything in regaurd to collecting and organizing
sensor data on the rocket.

This is highly flight-board specific, and is treated as such

There are no plans to make this easily modifiable for more sensors or equipment,
so everything is added and treated manually. This is done as to make development
easier.

Currently, the sensors on the V2 Falson Flight Altimeter are:
  ICM20948 (9 axis imu)
  BMP280 (Pressure + Temperature Sensor)

Future sensors include:
  100g accelerometer for high g launches
  GPS sensor


*/



#ifndef _SENSOR_PACKAGE_H_
#define _SENSOR_PACKAGE_H_

#include <BMP280.h>
#include <ICM20948.h>
#include <RocketState.h>
#include <Arduino.h>

#define UPDATE_PERFORMED_NEW_DATA_AVAILABLE   1
#define UPDATE_NOT_REQUIRED                   2

//errors
#define IMU_INIT_FAILURE                      -1
#define BMP_INIT_FAILURE                      -2

using namespace RocketHelper;

class SensorPackage{
public:
  int begin(float imuUpdateRate, float pressureUpdateRate);
  void setImuUpdateRate(int rate);
  void setPressureUpdateRate(int rate);
  int getData(RawSensorData *data);
  void pauseDataCollection();
  void startDataCollection();
  int update();

private:
  ICM20948 icm;
  BMP280 bmp;
  ICM20948::RawData icmData; //necessary for 'data transfer'
  RawSensorData rawData;

  //an update rate < 0 means uninitialized, 0 means as fast as possible, anything
  //else is attempted to match
  uint imuUpdateMillis = -1;
  uint bmpUpdateMillis = -1;

  //error tracking
  int mostRecentErrorCode = 0;
  bool bmpFailure = false;
  bool imuFailure = false;

  int initImu(int rate);
  int initBmp(int rate);
  int updateImu();
  int updateBmp();

  void copyImuData(ICM20948::RawData *source, RawImuData *target);


};


#endif
