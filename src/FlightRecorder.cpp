#include <FlightRecorder.h>

int FlightRecorder::init(DataRecorder *dataRecorder){
  this->dataRecorder = dataRecorder;

}

int FlightRecorder::setDataFrequency(int dataFlag, uint newFrequency){
  switch(dataFlag){
    case(RAW_BARO_TEMP_DATA_FLAG):
      data_recording_frequency.raw_baro_temp_frequency = newFrequency;
      rawBaroTempUpdater.setUpdateFrequency(newFrequency);
      break;

    case(RAW_ACCEL_DATA_FLAG):
      data_recording_frequency.raw_accel_frequency = newFrequency;
      rawAccelUpdater.setUpdateFrequency(newFrequency);
      break;

    case(RAW_GYRO_DATA_FLAG):
      data_recording_frequency.raw_gyro_frequency = newFrequency;
      rawGyroUpdater.setUpdateFrequency(newFrequency);
      break;

    case(RAW_MAG_DATA_FLAG):
      data_recording_frequency.raw_mag_frequency = newFrequency;
      rawMagUpdater.setUpdateFrequency(newFrequency);
      break;

    default:
      //not handled
      return 1; //'soft' error
      break;
  }
  return 0;
}

int FlightRecorder::update(RocketState *newState){
  if(recordingState != PAUSED && recordingState != FLIGHT_COMPLETED){
    //'normal' operation
    //go through each updater and update if necessary
    if(rawGyroUpdater.update() == TIME_UPDATER_UPDATE_REQUIRED){
      dataRecorder.recordStateData(RAW_GYRO_DATA_FLAG, newState);
    }
    if(rawAccelUpdater.update() == TIME_UPDATER_UPDATE_REQUIRED){
      dataRecorder.recordStateData(RAW_ACCEL_DATA_FLAG, newState);
    }
    if(rawMagUpdater.update() == TIME_UPDATER_UPDATE_REQUIRED){
      dataRecorder.recordStateData(RAW_MAG_DATA_FLAG, newState);
    }
    if(rawBaroTempUpdater.update() == TIME_UPDATER_UPDATE_REQUIRED){
      dataRecorder.recordStateData(RAW_BARO_TEMP_DATA_FLAG, newState);
    }
  }
  else{
    //abnormal operation, we don't really do anything here... for now...
    return FLIGHT_RECORDER_RECORDING_NOT_NECESSARY;
  }
}

int FlightRecorder::stop_recording(){
  //stop updating
}
