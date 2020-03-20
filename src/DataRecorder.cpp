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
  writeBuf[3] = (uint8_t)((timestamp >> 16)%256);
  writeBuf[4] = (uint8_t)((timestamp >> 8)%256);
  writeBuf[5] = (uint8_t)(timestamp % 256);
  for(uint i = 0; i < size; i ++){
    writeBuf[6+i] = buffer[i];
  }
  //write this now
  #ifdef SERIAL_WRITE_DEBUG_STORAGE
    Serial.print("\nDataRecorder Writing: Flag: " + String(flag) + "; Timestamp: " + String(timestamp) + "; Buffer: ");
    for(uint i = 0; i < (6 + size); i ++){
      Serial.print(String(writeBuf[i]) + " ");
    }
    Serial.println();
  #endif
  int error = 0;
  #ifndef DEBUG_DATA_STORAGE
    error = storage.write(&writeBuf[0],6+size);
  #endif
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_WRITE_ERROR;
  }
  return 0;
}

int DataRecorder::recordStateData(int dataFlag, RocketState *state){
  //TODO handle
  int error = 0;
  switch(dataFlag){
    case(RAW_BARO_TEMP_DATA_FLAG):
      error = record_raw_baro_temp_data(&state->rawSensorData.pressureData);
      break;

  }
  if(error < 0){
    //already have an error passthrough, don't use the typical lastError format
    return error;
  }
  return 0;
}

int DataRecorder::scale_data(float value, byte *buf, uint length, float minVal, float maxVal){
  int error = 0;
  float min_offset_value = value - minVal;
  if(min_offset_value < 0){
    min_offset_value = 0;
    error = DATA_RECORDING_DATA_LESS_THAN_MIN_VAL;
  }
  else if(min_offset_value > maxVal - minVal){
    min_offset_value = maxVal - minVal;
    error = DATA_RECORDING_DATA_GREATER_THAN_MAX_VAL;
  }
  float scale = (maxVal - minVal) / pow(2,8 * length);
  unsigned long a = (unsigned long)(min_offset_value / scale + .5);
  if (a > pow(2,8*length)){
    a = pow(2,8*length);
  }
  for(int i = 0; i < length; i ++){
    buf[i] = (a >> ((length - i - 1) * 8) ) % 256;
  }
  return error;
}

int DataRecorder::record_raw_baro_temp_data(RawPressureData *data){
  //scale data
  byte dataBuffer[RAW_BARO_SIZE];
  int error = scale_data(data->pressure, &dataBuffer[0], RAW_BARO_SIZE, MIN_PRESSURE, MAX_PRESSURE);
  for(int i = 0; i < RAW_BARO_SIZE; i ++){
    Serial.print(String(dataBuffer[i]) + " ");
  }
  Serial.println();
  if(error < 0){
    lastError = error;
    return DATA_RECORDING_BARO_TEMP_DATA_RECORD_ERROR;
  }
  //go and write the data to storage
  error = write_data(RAW_BARO_TEMP_DATA_FLAG,data->timestamp,&dataBuffer[0],RAW_BARO_SIZE);
  if(error < 0){
    lastError = error;
    return DATA_WRITE_ERROR;
  }
  return 0;
}
