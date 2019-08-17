#include <SmallFAT.h>

#define SERIAL_WRITE
#define IO_WRITE

int SmallFAT::init(int cs_pin, bool override_previous_table){
  //check the storage medium
  int status = storageMedium.init(cs_pin, true);
  if(status < 0){
    #ifdef SERIAL_WRITE
      Serial.println("Storage Fail: " + String(status));
    #endif
    return STORAGE_MEDIUM_INIT_FAILURE;
  }
  //go grab the file allocation table
  if(!override_previous_table){
    status = getFileAllocationTable();
    if(status == NO_FAT_TABLE_FOUND){
      //make one!
      //or nah...
      #ifdef SERIAL_WRITE
        Serial.println("No FAT found!");
      #endif
    }
  }
  return 0;
}

int SmallFAT::getFileAllocationTable(FileAllocationTable * table){
  //copy the master into
  table->num_files = masterTable.num_files;
  for(uint i = 0; i < table->num_files; i ++){
    table->info[i].start_page = masterTable.info[i].start_page;
    table->info[i].end_page = masterTable.info[i].end_page;
    table->info[i].end_offset = masterTable.info[i].end_offset;
  }
  return 0;
}

int SmallFAT::eraseLastFile(){
  //remove from the masterTable
  masterTable.num_files --;
  masterTable.info[masterTable.num_files].start_page = 0;
  masterTable.info[masterTable.num_files].end_page = 0;
  masterTable.info[masterTable.num_files].end_offset = 0;
  writeFileAllocationTable();
  return 0;
}

int SmallFAT::getFileAllocationTable(){
  //go read the pages until the 0s
  //if no 0s, assume no file allocation table
  byte fat_read_buffer[256];
  int status = waitUntilFree();
  if(status != 0) return status;
  storageMedium.read256(0, &fat_read_buffer[0]);
  //the first byte records the number of files available
  uint filesAvail = fat_read_buffer[0];
  #ifdef SERIAL_WRITE
    Serial.println("files: " + String(filesAvail));
  #endif
  if(filesAvail == 255){
    //no FAT table!
    return NO_FAT_TABLE_FOUND;
  }
  for(uint i = 0; i < filesAvail; i ++){
    masterTable.info[i].start_page = (uint16_t)(fat_read_buffer[1+i*5]) << 8 | fat_read_buffer[2+i*5];
    masterTable.info[i].end_page = (uint16_t)(fat_read_buffer[3+i*5]) << 8 | fat_read_buffer[4+i*5];
    masterTable.info[i].end_offset = fat_read_buffer[5+i*5];
    #ifdef SERIAL_WRITE
      Serial.println("Entry found! Start: " + String(masterTable.info[i].start_page) + "\t End: " + String(masterTable.info[i].end_page) + "\t Offset: " + String(masterTable.info[i].end_offset));
    #endif
  }
  masterTable.num_files = filesAvail;
  return 0;
}

int SmallFAT::open(FILE_MODE mode, uint fd){
  //make sure a file's not open
  if(cur_mode != STANDBY){
    close(); // for safety as this will likely cause a program halt
    return READ_WRITE_MODE_MISMATCH;
  }
  switch(mode){
  //otherwise, lets handle it
    case(WRITE):{
        //go add to the masterTable
        int status = makeNewFile();
        if(status < 0) return status;
        currentAddress = masterTable.info[masterTable.num_files - 1].start_page * 256;
        highestErasedAddress = currentAddress;
        break;
      }
    case(READ):
      if(fd >= masterTable.num_files){
        #ifdef SERIAL_WRITE
          Serial.println("FD out of range");
        #endif
        return FILE_DOES_NOT_EXIST;
      }
      currentFD = fd;
      //get the start addr and such
      currentAddress = masterTable.info[currentFD].start_page * 256;
      //Serial.println("Reading addr: " + String(currentAddress));
      break;
    default:
      break;
  }
  cur_mode = mode;
  return 0;
}

int SmallFAT::readStream(byte *buf, uint length){
  //read into the stream, return the number of bytes actually read
  ulong endAddr = masterTable.info[currentFD].end_page * 256 + masterTable.info[currentFD].end_offset;
  //Serial.println("End Addr: " + String(endAddr));
  if(currentAddress + length < endAddr){
    #ifdef IO_WRITE
      int status = storageMedium.read(currentAddress, buf, length);
      if(status != 0) return status;
    #endif
    #ifdef SERIAL_WRITE
      Serial.println("ReadStream!");
    #endif
    currentAddress += length;
    return length;
  }
  //otherwise, read up to max addr
  int readLength = endAddr - currentAddress;
  #ifdef IO_WRITE
    int status = storageMedium.read(currentAddress, buf, readLength);
    if(status != 0) return status;
  #endif
  #ifdef SERIAL_WRITE
    Serial.println("ReadStream!");
  #endif
  currentAddress += readLength;
  return readLength;
}

long SmallFAT::peak(){
  ulong endAddr = masterTable.info[currentFD].end_page * 256 + masterTable.info[currentFD].end_offset;
  if(endAddr > currentAddress) return endAddr - currentAddress;
  return 0;
}

int SmallFAT::makeNewFile(){
  //make sure there's room
  if(masterTable.num_files < MAX_FILES){
    //increment and set the start page
    //only can make a new file every 4kb
    ulong currentAddress = masterTable.info[masterTable.num_files - 1].end_page * 256;
    ulong nextAddress = (currentAddress/(1<<12) + 1) << 12;
    //Serial.println("Next Addr: " + String(nextAddress));
    masterTable.info[masterTable.num_files].start_page = nextAddress/256; //convert to page
    masterTable.info[masterTable.num_files].end_page = masterTable.info[masterTable.num_files].start_page;
    masterTable.num_files ++;
    //go write the table for future reference
    int status = writeFileAllocationTable();
    if(status < 0) return status;
    //go erase a sector and update it
    clearNextSector();
    return 0;
  }
  else{
    return STORAGE_FULL;
  }
}

int SmallFAT::writeFileAllocationTable(){
  //write the master table
  //convert data to buffer, do a one page buffer for now
  //start the sector erase, this may take some time
  //store the data in address of pages (256 bytes)
  int status = waitUntilFree();
  //Serial.println("There");
  if(status < 0) return status;
  //Serial.println("Here");
  #ifdef IO_WRITE
    storageMedium.eraseSector(0);
  #endif
  byte tempFatBuffer[256];
  tempFatBuffer[0] = masterTable.num_files;
  for(uint i = 0; i < masterTable.num_files; i ++){
    tempFatBuffer[1 + i*5] = masterTable.info[i].start_page >> 8;
    tempFatBuffer[2 + i*5] = (uint8_t)masterTable.info[i].start_page;
    tempFatBuffer[3 + i*5] = (uint8_t)(masterTable.info[i].end_page >> 8);
    tempFatBuffer[4 + i*5] = (uint8_t)masterTable.info[i].end_page;
    tempFatBuffer[5 + i*5] = masterTable.info[i].end_offset;
  }
  //fill the rest with blanks
  for(uint i = masterTable.num_files * 5 + 1; i < 256; i ++){
    tempFatBuffer[i] = 255;
  }
  //go write it
  //Serial.println("maybe!");
  status = waitUntilFree();
  if(status < 0) return status;
  //Serial.println("no..");
  #ifdef IO_WRITE
     status = storageMedium.write256(0, &tempFatBuffer[0]);
  #endif
  #ifdef SERIAL_WRITE
    Serial.println("FAT Table Write:" );
    for(uint i = 0; i < 16; i ++){
      for(uint p = 0; p < 16; p ++){
        Serial.print(String(tempFatBuffer[i*16 + p]) + "\t");
      }
      Serial.println();
    }
  #endif
  return status;
}

int SmallFAT::waitUntilFree(ulong timeoutMills){
  long start = millis();
  while(millis() - start < timeoutMills){
    if(!storageMedium.isBusy()) return 0;
  }
  //Serial.println("Elapsed Millis: " + String(millis() - start));
  return STORAGE_MEDIUM_TIMEMOUT_FAILURE;
}

int SmallFAT::clearNextSector(bool force){
  //actually, clear the sector... and update the remaining parameters
  //find the next 4kb addr
  if((highestErasedAddress - currentAddress <= 256) | force){
    //find the next available 4 kb
    long nextSectorAddr = ((currentAddress/(1<<12)) + 1) << 12;
    if(currentAddress%(1<<12) == 0){
      nextSectorAddr = currentAddress;
    }
    int status = waitUntilFree();
    if(status < 0) return status;
    #ifdef IO_WRITE
      status = storageMedium.eraseSector(nextSectorAddr);
    #else
      status = 0;
    #endif
    #ifdef SERIAL_WRITE
      Serial.println("Clearing sector at addr: " + String(currentAddress));
    #endif
    if(status < 0) return status;
    highestErasedAddress = nextSectorAddr;
  }
  return 0;
}

int SmallFAT::write(byte *buf, uint length){
  if(currentAddress + length > highestErasedAddress){
    clearNextSector();
  }
  if((length + write_cache_offset > 256) && (write_cache_offset != 0)){
    //go load in the data
    #ifdef SERIAL_WRITE
      Serial.println("Filling up previous buff");
    #endif
    for(uint i = write_cache_offset; i < 256; i ++){
      writeCache[i] = buf[i-write_cache_offset];
    }
    *buf = buf[256 - write_cache_offset]; //i think
    length -= 256 - write_cache_offset;
    int status = waitUntilFree();
    if(status < 0) return status;
    #ifdef IO_WRITE

      storageMedium.write256(currentAddress, &writeCache[0]);
    #endif
    #ifdef SERIAL_WRITE
      for(uint i = 0; i < 16; i ++){
        for(uint p = 0; p < 16; p ++){
          Serial.print(String(writeCache[i*16 + p]) + "\t");
        }
        Serial.println();
      }
    #endif
    currentAddress += 256;
    write_cache_offset = 0;
  }
  //now write up till we have less than 256 left
  while(length >= 256){
    #ifdef SERIAL_WRITE
      Serial.println("Writing bulk");
    #endif
    int status = waitUntilFree();
    if(status < 0) return status;
    #ifdef IO_WRITE
      storageMedium.write256(currentAddress, &buf[0]);
    #endif
    #ifdef SERIAL_WRITE
      for(uint i = 0; i < 16; i ++){
        for(uint p = 0; p < 16; p ++){
          Serial.print(String(buf[i*16 + p]) + "\t");
        }
        Serial.println();
      }
    #endif
    buf = &buf[256];
    length -= 256;
    currentAddress += 256;
  }
  //lastly, fill the rest of the write cache
  for(uint i = 0; i < length; i ++){
    writeCache[i + write_cache_offset] = buf[i];
  }
  write_cache_offset += length;
  //update the fat table to reflect the current state in case of emergency
  masterTable.info[masterTable.num_files-1].end_page = currentAddress/256;
  writeFileAllocationTable();
  return 0;
}

int SmallFAT::forceWrite(){
  //should only be done when closing a file!!
  //force a write of the buffer, if applicable
  if(cur_mode != WRITE) return READ_WRITE_MODE_MISMATCH;
  if(write_cache_offset == 0) return 0;
  //fill the reamaining buffer with '255'
  for(uint i = write_cache_offset; i < 256; i ++){
    writeCache[i] = 255;
  }
  int status = waitUntilFree();
  if(status < 0) return status;
  #ifdef IO_WRITE
    storageMedium.write256(currentAddress, &writeCache[0]);
  #endif
  #ifdef SERIAL_WRITE
    for(uint i = 0; i < 16; i ++){
      for(uint p = 0; p < 16; p ++){
        Serial.print(String(writeCache[i*16 + p]) + "\t");
      }
      Serial.println();
    }
  #endif
  currentAddress += 256;
  //set the offset
  masterTable.info[masterTable.num_files-1].end_offset = write_cache_offset;

  return 0;
}

int SmallFAT::close(bool saveFile){
  switch(cur_mode){
    case(WRITE):
      //finish writing the write cache
      if(saveFile){
        forceWrite();
      }
      else{
        Serial.println("Num Files Pre Wipe: " + String(masterTable.num_files));
        masterTable.num_files --;
        Serial.println("Num Files Post Wipe: " + String(masterTable.num_files));
      }
      writeFileAllocationTable();
      getFileAllocationTable();
      break;
    case(READ):
      //not really anything to do..
      break;
    case(STANDBY):
      //uhhhh....
      break;
  }
  currentFD = 0;
  cur_mode = STANDBY;
  currentAddress = 0;
  return 0;
}
