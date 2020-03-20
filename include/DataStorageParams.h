/*
  DataStorageParams

  Defines the scales and extra offsets used to encode (and decode) data
  There is limited storage space, so the data is scaled (losing resolution) to fit in the available storage space

*/


#ifndef _DATA_STORAGE_PARAMS_H
#define _DATA_STORAGE_PARAMS_H

// sizes are in bytes

#define RAW_BARO_SIZE 2
#define RAW_TEMP_SIZE 2
#define MAX_PRESSURE 1500000
#define MIN_PRESSURE 0


#define RAW_GYRO_SIZE 2
#define MIN_GYRO -2000
#define MAX_GYRO 2000

#define RAW_ACCEL_SIZE 2
#define MIN_ACCEL -16
#define MAX_ACCEL 16

#define RAW_MAG_SIZE 2
#define MIN_MAG 0
#define MAX_MAG 10000

#define ALTITUDE_SIZE 3
#define MAX_ALTITUDE 30000
#define MIN_ALTITUDE -1000

#define MAX_VELOCITY 3430
#define MIN_VELOCITY -3430
#define VELOCITY_SIZE 3

#define ATTITUDE_SIZE 2
#define MIN_ATTITUDE 0
#define MAX_ATTITUDE 360

#define TIME_SIZE 3
#define MAX_TIME 16777215




#endif
