#ifndef _SMALL_FAT_H_
#define _SMALL_FAT_H_

#include <Arduino.h>
#include <W25Q64FV.h>

#define SMALL_FAT_PAGE_LENGTH 256
#define MAX_FILES 128

#define WRITE_CACHE_SIZE 256
#define STANDARD_TIMEMOUT_MILLIS 1000


//soft failures
#define NO_FAT_TABLE_FOUND 10

//hard failures
#define STORAGE_MEDIUM_INIT_FAILURE -10
#define STORAGE_FULL -11
#define STORAGE_MEDIUM_TIMEMOUT_FAILURE -12
#define READ_WRITE_MODE_MISMATCH -20
#define FILE_DOES_NOT_EXIST -25


class SmallFAT{
public:
  enum FILE_MODE{
    READ, WRITE, STANDBY
  };

  struct FileInfo{
    uint start_page = 0;
    uint end_page = 0;
    uint end_offset = 0;
  };

  struct FileAllocationTable{
    FileInfo info[MAX_FILES];
    uint num_files = 0;
  };

  int init( int cs_pin = PA4, bool override_previous_table = false);
  int open(FILE_MODE mode, uint fd = MAX_FILES + 1);
  int readStream(byte *buf, uint length);
  int write(byte *buf, uint length);
  int close(bool saveFile = true);
  int getFileAllocationTable(FileAllocationTable *target);
  int eraseLastFile();
  long peak();

private:
  FileAllocationTable masterTable;
  int getFileAllocationTable();
  int clearNextSector(bool force = false);
  int waitUntilFree(ulong timeoutMills = STANDARD_TIMEMOUT_MILLIS);
  int createFATtable();
  int writeFileAllocationTable();
  int makeNewFile();
  int forceWrite();
  FILE_MODE cur_mode = STANDBY;

  uint currentFD = 0;
  long currentAddress = 0;
  long highestErasedAddress = 0;
  byte writeCache[WRITE_CACHE_SIZE];
  uint8_t write_cache_offset = 0;
  W25Q64FV storageMedium;

};

#endif
