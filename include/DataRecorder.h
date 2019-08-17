/*
DataRecorder

Handles encoding and decoding data into the storage medium
Handles all interaction with the storage medium as well

Data is encoded using a 2 byte data flag, 1 byte data length (in bytes), 3 byte timestamp, and n bytes of data

Flags are defined below

timestamp is recorded in milliseconds, and thus has a max time of 16777216 seconds, or 4.66 hours
time will be rolled over after that

the data length flag is done for backwards and forward compatibility, so data with unkown flags can be ignored by and post readers

Data is written in 256 byte pages, so there is potential for loss of data. However, this is a small enough block that this is acceptable

Current storage medium is a 'faked' FAT system running on a flash storage medium. This is used for storing flight data on multiple flights



Reading back data is done in the raw form (given above) and is up to another parser to parse (likely not on the flightcomputer's end).
No reverse parsing is preformed by the controller (with the exception of flight programming)
*/

#ifndef _DATA_RECORDER_H_
#define _DATA_RECORDER_H_

#include <RocketState.h>
#include <DataFlags.h>
#include <SmallFAT.h>
#include <Arduino.h>

#define DATA_RECORDING_STORAGE_INIT_FAILURE -1
#define DATA_RECORDING_NO_MORE_FILES -2
#define DATA_RECORDING_CONFIG_FILE_PREMAUTRE_DELETION_ATTEMPT -3
#define DATA_RECORDING_READ_ERROR -4
#define DATA_RECORDING_FILE_DOES_NOT_EXIST -5
#define DATA_RECORDING_FILE_CLOSE_ERROR -6
#define DATA_RECORDING_WRITE_ERROR -7

class DataRecorder{
public:

  /*
    Used to communicate the availble files and sizes
  */

  struct FileList{
    unsigned long sizes[256];
    uint8_t num_files;
  };

  /*
    init

    initializes the DataRecorder and Data Storage Medium (in this case the flash chip)

    @return int status code based upon init success (0) or failure (<0) or anything else (>0)
  */
  int init();

  /*
    get_file_list

    gets and returns the available files and sizes on the storage medium

    @return FileList available files and sizes
  */
  FileList get_file_list();

  /*
    delete_last_file

    Deletes the last file (removes listing from FAT table). Currently, a simple FAT tables acts like a FILO buffer
    and thus needs to be deleted last to first flight recording

    @return int available files still in storage (if > 0) or an error if < 0
  */
  int delete_last_file();

  /*
    delete_config_file

    Deletes the config file (fd=0). This is an extra operation to protect the device from accidental erase
    It is highly recommended this is done followed by a config file creation
    This can only be done following deletion of the entire storage medium

    @return status code of erase failure.
  */
  int delete_config_file();

  /*
    delete_all_files

    Deletes all the storage files from the storage medium

    @return int status code, < 0 failure, 0 is success

  */
  int delete_all_files();

  /*
    new_file

    Initializes a new file to allow for flight data to be stored in

    @return status code of success or failure

  */
  int new_file();

  /*
    close_file

    Closes an open file (whether a file open for reading or writing)

    @return status code of success or failure
  */
  int close_file();

  /*
    open_file

    Opens a file for reading based on its file descripter

    @param fd a file descripter for the desired file to read
    @return status code of the success of failure of openning the file

  */
  int open_file(int fd);

  /*
    read_next

    Returns the next grouping of data (dependent on the size of the segment, see header)

    @param buffer pointer to a buffer to store the next set of data
    @param length the maximum size of buffer (should default to the max file size, ~256 bytes for now)
    @return the size of data in buffer (if > 0) or a failure code
  */
  int read_next(byte *buffer, uint length);

  /*
    peek

    checks to see if more data is available and returns the size of the next block

    @return int size of the next available data grouping (if > 0), 0 if no more data is left, < 0 an error code
  */
  int peek();

  /*
    Record State Data

    Records one data peice based on the data flag

    @param dataFlag data to be recorded, see DataFlags.h
    @param state state to get the data from
    @return int code based on success or failure
  */
  int recordStateData(int dataFlag, RocketState *state);


  int record_flight_phase_change(RocketFlightPhase *newPhase, unsigned long timestamp);
  int record_drogue_deployment(unsigned long timestamp);
  int record_main_deployment(unsigned long timestamp);

  int get_error();

private:
  int lastError = 0;
  SmallFAT storage;

  int write_data(uint flag, unsigned long timestamp, byte *buffer, uint size);
  int init_storage_medium();

  // data recording functions. One per data type (for ease of reading). these functions also handle data scaling
  int record_raw_imu_data(RawImuData *data);
  int record_raw_baro_temp_data(RawPressureData *data);
  int record_filtered_alt_data(FilteredAltData *data);
  int record_filtered_vel_data(FilteredVelData *data);
  //int record_filtered_attitude_data();  //TODO




};

#endif
