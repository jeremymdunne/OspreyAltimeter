#ifndef _ROCKET_STATE_
#define _ROCKET_STATE_

//define the state of the rocket through structures

struct RawImuData{
  float accel[3] = {0,0,0};
  float gyro[3] = {0,0,0};
  float mag[3] = {0,0,0};
  float temperature = 0;
  unsigned long timestamp = 0;
};

struct RawPressureData{
  float pressure = 0;
  float temperature = 0;
  unsigned long timestamp = 0;
};

struct RawSensorData{
  RawImuData imuData;
  RawPressureData pressureData;

};

struct FilteredAltData{
  float altitude = 0;
  float accuracy = 0;
  unsigned long timestamp = 0;
  //TODO flags to denote method, model used to obtain
};

struct FilteredVelData{
  float velocity = 0;
  float accuracy = 0;
  unsigned long timestamp = 0;
  //TODO flags to denote method, model used to obtain
};

enum RocketFlightPhase{
  STANDBY, INITIALIZE_ERROR, WAITING_FOR_LAUNCH, BOOST_PHASE, COAST_PHASE, APOGEE_PHASE, RECOVERY_PHASE, LANDING_PHASE, LANDED
};

struct RocketState{
  // define all data associated with the rocket here
  unsigned long timeReference = 0;
  RocketFlightPhase currentFlightPhase = STANDBY;

  //sensors
  RawSensorData rawSensorData;
  //TODO GPS Maybe

  unsigned long flight_start_timestamp = 0;

};

namespace RocketHelper{

  static void copyRawImuData(RawImuData *source, RawImuData *target){
    for(int i = 0; i < 3; i ++){
      target->accel[i] = source->accel[i];
      target->gyro[i] = source->gyro[i];
      target->mag[i] = source->mag[i];
    }
    target->temperature = source->temperature;
    target->timestamp = source->timestamp;
  }

  static void copyPressureData(RawPressureData *source, RawPressureData *target){
    target->pressure = source->pressure;
    target->temperature = source->temperature;
    target->timestamp = source->timestamp;
  }

  static void copyRawSensorData(RawSensorData *source, RawSensorData *target){
    copyRawImuData(&source->imuData, &target->imuData);
    copyPressureData(&source->pressureData, &target->pressureData);
  }


}

#endif
