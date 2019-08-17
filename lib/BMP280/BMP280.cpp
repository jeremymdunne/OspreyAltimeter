#include <BMP280.h>
int BMP280::begin(int address, bool initWire){
  bmpAddress = address;
  if(initWire){
    Wire.begin();
    Wire.setClock(400000);
  }
  if(checkID(true) == 0){
    //get calibration data
    getCalibrationValues(&calibrationValues);
    setPressureOversampling(PRESSURE_OVERSAMPLING_4X);
    setTemperatureOversampling(TEMPERATURE_OVERSAMPLING_1X);
    setStandbyTime(STANDBY_HALF_M_SEC);
    //set normal mode
    setMode();

    return 0;
  }
  return 1;
}

int BMP280::readSensor(float *temperature, float *pressure){
  //grab all the datas
  write8(BMP180_PRESSURE_MSB_ADDRESS);
  Wire.requestFrom(bmpAddress, 6);
  int32_t pressureData = 0;
  int32_t temperatureData = 0;
  if(Wire.available()){
    pressureData = (int32_t)(Wire.read());
    pressureData <<= 8;
    pressureData |= Wire.read();
    pressureData <<= 8;
    pressureData |= Wire.read();
    //shift back 4
    pressureData >>= 4;
    temperatureData = (int32_t)(Wire.read());
    temperatureData <<= 8;
    temperatureData |= Wire.read();
    temperatureData <<= 8;
    temperatureData |= Wire.read();
    temperatureData >>= 4;
  }

  //go and do the appropriate compensations
  *temperature = compensateTemperature(temperatureData);
  *pressure = compensatePressure(pressureData);
  return 0;
}

  float BMP280::compensateTemperature(int32_t tempReading){
    //magical formula as defined in the data sheet
    int32_t var1, var2, T;
    var1 = ((((tempReading>>3) - ((int32_t)calibrationValues.dig_T1<<1))) * ((int32_t)calibrationValues.dig_T2)) >> 11;
    var2 = (((((tempReading>>4) - ((int32_t)calibrationValues.dig_T1)) * ((tempReading>>4) - ((int32_t)calibrationValues.dig_T1))) >> 12) * ((int32_t)calibrationValues.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T/100.0;
  }
  float BMP280::compensatePressure(int32_t pressureReading){
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calibrationValues.dig_P6;
    var2 = var2 + ((var1*(int64_t)calibrationValues.dig_P5)<<17);
    var2 = var2 + (((int64_t)calibrationValues.dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)calibrationValues.dig_P3)>>8) + ((var1 * (int64_t)calibrationValues.dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)calibrationValues.dig_P1)>>33;
    if (var1 == 0) {
      return 0; // avoid exception caused by division by zero
    }
    p = 1048576-pressureReading;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)calibrationValues.dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)calibrationValues.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calibrationValues.dig_P7)<<4);
    return (float)(p/256.0);
  }

  int BMP280::getCalibrationValues(BMP180_Calibration_Values *values){
    //per the data sheet, read these values in. Keep in mid their data type
    //all are 16 bit shorts
    write8(BMP180_CALIBRATION_DIG_T1_ADDRESS);
    //request all 24 adresses
    Wire.requestFrom(bmpAddress, 24);
    //read in 1 at a time
    if(Wire.available()){
      values->dig_T1 = (u_int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_T2 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_T3 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P1 = (u_int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P2 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P3 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P4 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P5 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P6 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P7 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P8 = (int16_t)(Wire.read()|(Wire.read()<<8));
      values->dig_P9 = (int16_t)(Wire.read()|(Wire.read()<<8));
      return 0;
    }
    return BMP180_CALIBRATION_DATA_READ_FAILURE;
  }

  int BMP280::checkID(bool shouldReset){
    //first reset the device if desired
    if(shouldReset){
      int resetStatus = reset();
      if(resetStatus != 0){
        return BMP180_CONNECTION_ISSUE;
      }
    }
    //get the address of the device
    int resp = read8(BMP180_ID_ADDRESS);
    //compare, if correct, return 0
    Serial.println(resp);
    if(resp == BMP180_WHO_AM_I_RESPONSE){
      return 0;
    }
    //return failure values as required
    if(resp == -1){
      return BMP180_CONNECTION_ISSUE;
    }
    else{
      return BMP180_BAD_WHO_AM_I_ADDRESS;
    }
    return 1;
  }

  int BMP280::reset(){
    //reset, no easy way to check for alive
    write8(bmpAddress, BMP180_RESET_WRITE_VALUE);
    delay(1);
    return 0;
  }

  int BMP280::setTemperatureOversampling(BMP180_Temperature_Oversampling sample){
    //first read the data in the register
    int settings = read8(BMP180_CTRL_MEAS_ADDRESS);
    //combine oversampling into the current settings
    settings &= 0b00011111;
    settings |= sample << 5;
    //update the currentSampling
    //currentSampling = sample;
    //send back to the BMP
    return write8(BMP180_CTRL_MEAS_ADDRESS, settings);
  }

  int BMP280::setStandbyTime(BMP280_Standby_Time time){
    //first read the data in the register
    int settings = read8(BMP180_CONFIG_ADDRESS);
    //combine oversampling into the current settings
    settings &= 0b00011111;
    settings |= time << 5;
    //update the currentSampling
    //currentSampling = sample;
    //send back to the BMP
    return write8(BMP180_CONFIG_ADDRESS, settings);
  }

  int BMP280::setPressureOversampling(BMP180_Pressure_Oversampling sample){
    //first read the data in the register
    int settings = read8(BMP180_CTRL_MEAS_ADDRESS);
    //combine oversampling into the current settings
    settings &= 0b11100011;
    settings |= sample << 2;
    //update the currentSampling
    //currentSampling = sample;
    //send back to the BMP
    return write8(BMP180_CTRL_MEAS_ADDRESS, settings);
  }

  int BMP280::setMode(){
    //go to normal for now
    //read the settings in the control
    int settings = read8(BMP180_CTRL_MEAS_ADDRESS);
    settings &= 0b11111100;
    settings |= 0b00000011;
    //write settings
    write8(BMP180_CTRL_MEAS_ADDRESS,settings);
    return 0;
  }

  int BMP280::setFilter(BMP180_Filter_Coefficients coefficient){
    //first read the data in the register
    int settings = read8(BMP180_CONFIG_ADDRESS);
    //combine oversampling into the current settings
    settings &= 0b11100011;
    settings |= coefficient << 2;
    //update the currentSampling
    //currentSampling = sample;
    //send back to the BMP
    return write8(BMP180_CONFIG_ADDRESS, settings);
  }


    int BMP280::read8(int address){
      Wire.beginTransmission(bmpAddress);
      Wire.write(address);
      Wire.endTransmission();
      Wire.requestFrom(bmpAddress, 1);
      if(Wire.available()){
        return Wire.read();
      }
      return -1;
    }

    int BMP280::write8(int address, int value){
      Wire.beginTransmission(bmpAddress);
      Wire.write(address);
      Wire.write(value);
      Wire.endTransmission();
      return 0;
    }

    int BMP280::write8(int address){
      Wire.beginTransmission(bmpAddress);
      Wire.write(address);
      Wire.endTransmission();
      return 0;
    }
