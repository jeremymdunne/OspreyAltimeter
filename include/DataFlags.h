/*
DataFlags

Data Flags are used as a means of communicating and storing data for future recall.
Flags are used as identifiers of data types.

Event Flags are likewise used in this manner

*/



#ifndef _DATA_FLAGS_H_
#define _DATA_FLAGS_H_



//Flags
//Data members
#define RAW_BARO_TEMP_DATA_FLAG 2
#define FILTERED_ALT_DATA_FLAG 3
#define FILTERED_VEL_DATA_FLAG 4
#define FILTERED_ATTITUDE_DATA_FLAG 5
#define RAW_GYRO_DATA_FLAG 6
#define RAW_ACCEL_DATA_FLAG 7
#define RAW_MAG_DATA_FLAG 8

//Events
#define LAUNCH_DETECTION_FLAG 30
#define ENGINE_CUTOFF_DETECTION_FLAG 31
#define APOGEE_DETECTION_FLAG 32
#define DROGUE_DEPLOYMENT_FLAG 33
#define MAIN_DEPLOYMENT_FLAG 34
#define LANDING_FLAG 35


//flight parameters
#define GENERAL_FLIGHT_CHARACTERISTICS_FLAG 100
#define PYRO_ONE_FLAG 101
#define PYRO_TWO_FLAG 102
#define PYRO_THREE_FLAG 103
#define PYRO_FOUR_FLAG 104


//miscelaneous
#define TIME_ROLL_OVER_FLAG 350



#endif
