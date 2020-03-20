/*
FlightRecorder

Handles determining when and what data to record, and passes data to the Data Recorder

Compares the current RocketState to the previous to look for vast differences, or when
other triggers have been meet (i.e. sudden jerk, atttitude change, or event). Most data
collection is based on frequency triggers and attempts to match the desired frequency.

This class is bypassed during data recollection (i.e. Data recorder is directly accessed when reading from files)
This is done when hooked up to a master attempting to recall data, as well as when data about expected flight characterisitcs,
if used, are examined by the kinematic engine on initialization.

Because of this, a pointer to the DataRecorder class is a parameter of the init function


Flight Recorder will also be responsible for caching data before launch detection

*/



#ifndef _FLIGHT_RECORDER_H_
#define _FLIGHT_RECORDER_H_

#include <DataFlags.h>
#include <RocketState.h>
#include <UpdateScheduler.h>
#include <DataRecorder.h>


#define PRE_LAUNCH_RECORDING_BUFFER 512
#define FLIGHT_RECORDER_RECORDING_NOT_NECESSARY 1


class FlightRecorder{
public:

  enum FlightRecordingState{
    STARTUP_STANDBY, PRE_LAUNCH_RECORDING, FLIGHT_RECORDING, LANDING_RECORDING,
    FLIGHT_COMPLETED, PAUSED
  };

  //fill with basic data
  struct DataRecordingFrequency{
    uint raw_gyro_frequency = 10;
    uint raw_accel_frequency = 10;
    uint raw_mag_frequency = 10;
    uint raw_baro_temp_frequency = 10;

    uint filtered_attitude_frequency = 2;
    uint filtered_alt_frequency = 3;
    uint filtered_vel_frequency = 4;
  };

  int init(DataRecorder *dataRecorder);

  int setDataFrequency(int dataFlag, uint newFrequency);

  int start_pre_launch_recording();

  int save_pre_launch_recording();

  int start_recording();

  int pause_recording();

  int stop_recording();

  int update(RocketState *newState);


private:
  DataRecordingFrequency data_recording_frequency;
  RocketState previous_state;
  UpdateScheduler rawGyroUpdater, rawAccelUpdater, rawMagUpdater, rawBaroTempUpdater;
  DataRecorder *dataRecorder;
  FlightRecordingState recordingState = STARTUP_STANDBY;

};


#endif
