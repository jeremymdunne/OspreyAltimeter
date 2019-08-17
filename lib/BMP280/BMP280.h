#ifndef BMP_280_H_
#define BMP_280_H_
#include <Arduino.h>
#include <Wire.h>


#define BMP180_TEMP_XLSB_ADDRESS 0xFC
#define BMP180_TEMP_LSB_ADDRESS 0xFB
#define BMP180_TEMP_MSB_ADDRESS 0xFA
#define BMP180_PRESSURE_XLSB_ADDRESS 0xF9
#define BMP180_PRESSURE_LSB_ADDRESS 0xF8
#define BMP180_PRESSURE_MSB_ADDRESS 0xF7
#define BMP180_CONFIG_ADDRESS 0xF5
#define BMP180_CTRL_MEAS_ADDRESS 0xF4
#define BMP180_STATUS_ADDRESS 0xF3
#define BMP180_RESET_ADDRESS 0xE0
#define BMP180_ID_ADDRESS 0xD0
#define BMP180_CALIBRATION_START_ADDRESS 0xA1
#define BMP180_CALIBRATION_DIG_T1_ADDRESS 0x88
#define BMP180_CALIBRATION_DIG_T2_ADDRESS 0x8A
#define BMP180_CALIBRATION_DIG_T3_ADDRESS 0x8C
#define BMP180_CALIBRATION_DIG_P1_ADDRESS 0x8E
#define BMP180_CALIBRATION_DIG_P2_ADDRESS 0x90
#define BMP180_CALIBRATION_DIG_P3_ADDRESS 0x92
#define BMP180_CALIBRATION_DIG_P4_ADDRESS 0x94
#define BMP180_CALIBRATION_DIG_P5_ADDRESS 0x96
#define BMP180_CALIBRATION_DIG_P6_ADDRESS 0x98
#define BMP180_CALIBRATION_DIG_P7_ADDRESS 0x9A
#define BMP180_CALIBRATION_DIG_P8_ADDRESS 0x9C
#define BMP180_CALIBRATION_DIG_P9_ADDRESS 0x9F


#define BMP180_WHO_AM_I_RESPONSE 0x58
#define BMP180_RESET_WRITE_VALUE 0xB6
#define BMP180_ADDRESS_1 0x76
#define BMP180_ADDRESS_2 0x77
#define BMP180_CONNECTION_ISSUE 101
#define BMP180_BAD_WHO_AM_I_ADDRESS 102
#define BMP180_CALIBRATION_DATA_READ_FAILURE 103
#define BMP180_UPDATE_NOT_REQUIRED 104


class BMP280{
public:
  enum BMP180_Filter_Coefficients{
   FILTER_OFF = 0x00,
   FILTER_2 = 0x01,
   FILTER_4 = 0x02,
   FILTER_8 = 0x03,
   FILTER_16 = 0x04
  };

  enum BMP180_Temperature_Oversampling{
    TEMPERATURE_OVERSAMPLING_Skip = 0x00,
    TEMPERATURE_OVERSAMPLING_1X = 0x01,
    TEMPERATURE_OVERSAMPLING_2X = 0x02,
    TEMPERATURE_OVERSAMPLING_4X = 0x03,
    TEMPERATURE_OVERSAMPLING_8X = 0x04,
    TEMPERATURE_OVERSAMPLING_16X = 0x05
  };

  enum BMP180_Pressure_Oversampling{
    PRESSURE_OVERSAMPLING_SKIP = 0x00,
    PRESSURE_OVERSAMPLING_1X = 0x01,
    PRESSURE_OVERSAMPLING_2X = 0x02,
    PRESSURE_OVERSAMPLING_4X = 0x03,
    PRESSURE_OVERSAMPLING_8X = 0x04,
    PRESSURE_OVERSAMPLING_16X = 0x05
  };

  enum BMP280_Standby_Time{
    STANDBY_HALF_M_SEC = 0x00,
    STANDBY_63_M_SEC = 0x01,
    STANDBY_125_M_SEC = 0x02,
    STANDBY_250_M_SEC = 0x03,
    STANDBY_500_M_SEC = 0x04,
    STANDBY_1000_M_SEC = 0x05,
    STANDBY_2000_M_SEC = 0x06,
    STANDBY_4000_M_SEC = 0x07
  };

  int begin(int address = BMP180_ADDRESS_2, bool initWire = true);
  int readSensor(float *temperature, float *pressure);
  int reset();
  int setTemperatureOversampling(BMP180_Temperature_Oversampling sample);
  int setPressureOversampling(BMP180_Pressure_Oversampling sample);
  int setMode();
  int setFilter(BMP180_Filter_Coefficients coefficient);
  int setStandbyTime(BMP280_Standby_Time time);

private:
  int bmpAddress;
  struct BMP180_Calibration_Values{
    u_int16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    u_int16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
  };

  int read8(int address);
  int write8(int address, int value);
  int write8(int address);
  float compensateTemperature(int32_t tempReading);
  float compensatePressure(int32_t pressureReading);
  int getCalibrationValues(BMP180_Calibration_Values *values);
  int checkID(bool shouldReset = true);

  BMP180_Calibration_Values calibrationValues;
  int32_t t_fine;
  int32_t currentPressureData = 0;
  int32_t currentTemperatureData = 0;
  float previousTemperature = 0;
  float previousPressure = 0;
  float previousAltitude = 0;
  bool newPressureReported = false;
  bool newTemperatureReported = false;
  bool newAltitudeReported = false;
  long lastUpdateMillis = 0;
};

#endif
