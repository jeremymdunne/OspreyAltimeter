#include <DataRecorder.h>

int DataRecorder::init(){
  int error = init_storage_medium();
  if(error != 0){
    lastError = error;
    return DATA_RECORDING_STORAGE_INIT_FAILURE;
  }
  return 0;
}

DataRecorder::FileList DataRecorder::get_file_list(){
  FileList list;
  SmallFAT::FileAllocationTable table;
  storage.getFileAllocationTable(&table);
  for(int i = 0; i < (int)table.num_files; i ++){
    list.sizes[i] = (table.info[i].end_page - table.info[i].start_page * 256) + table.info[i].end_offset;
  }
  list.num_files = table.num_files;
  return list;
}

int DataRecorder::delete_last_file(){
  FileList list = get_file_list();
  if(list.num_files > 1){
    int error = storage.eraseLastFile();
    if(error < 0){
      lastError = error;
      return error;
    }
    return list.num_files - 1;
  }
  return DATA_RECORDING_NO_MORE_FILES;
}

int DataRecorder::delete_config_file(){
  FileList list = get_file_list();
  if(list.num_files == 1){
    int error = storage.eraseLastFile();
    if(error < 0){
      lastError = error;
      return error;
    }
    return 0;
  }
  return DATA_RECORDING_CONFIG_FILE_PREMAUTRE_DELETION_ATTEMPT;
}

int DataRecorder::delete_all_files(){
  //reset to catch errors
  lastError = 0;
  int status = delete_last_file();
  while(status != DATA_RECORDING_NO_MORE_FILES && lastError == 0){
    status = delete_last_file();
  }
  //try to catch if an error was thrown
  if(lastError != 0) return status;
  return 0;
}

int DataRecorder::init_storage_medium(){
  int error = storage.init(PA2);
  if(error != 0) return error;
  return 0;
}

int DataRecorder::get_error(){
  return lastError;
}

int DataRecorder::read_next(byte *buffer, uint length){
  int error = storage.readStream(buffer, 3);
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_READ_ERROR;
  }
  uint size = (uint8_t)buffer[2];
  error = storage.readStream(&buffer[3], size - 3);
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_READ_ERROR;
  }
  return size;
}

int DataRecorder::peek(){
  int size = storage.peak();
  if(size > 0) return 1;
  if(size == 0) return 0;
  return -1;
}

int DataRecorder::open_file(int fd){
  int error = storage.open(SmallFAT::READ,fd);
  if(error == FILE_DOES_NOT_EXIST){
    return DATA_RECORDING_FILE_DOES_NOT_EXIST;
  }
  return 0;
}

int DataRecorder::close_file(){
  int error = storage.close();
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_FILE_CLOSE_ERROR;
  }
  return 0;
}

int DataRecorder::write_data(uint flag, unsigned long timestamp, byte *buffer, uint size){
  // encode the data into the desired format
  byte writeBuf[6 + size]; // 2 for flag, 1 for size, 3 for time stamp
  writeBuf[0] = (uint8_t)(flag >> 8);
  writeBuf[1] = (uint8_t)(flag % 256);
  writeBuf[2] = size + 6;
  writeBuf[3] = (uint8_t)((flag >> 16)%256);
  writeBuf[4] = (uint8_t)((flag >> 8)%256);
  writeBuf[5] = (uint8_t)(flag % 256);
  for(int i = 0; i < size; i ++){
    writeBuf[6+i] = buffer[i];
  }
  //write this now
  int error = storage.write(&writeBuf[0],6+size);
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_WRITE_ERROR;
  }
  return 0;
}

int DataRecorder::recordStateData(int dataFlag, RocketState *state){
  //TODO handle
  return 0; 
}
