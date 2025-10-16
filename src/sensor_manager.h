#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include "utils.h"

struct SensorData {
    float ds18b20_temp;
    float ph_value;
    float do_value;
    float tds_value;
    float ammonia_value;
    float salinitas_value;
    float ec_value;
    String timestamp;
    bool ds18b20_error;
    bool ph_error;
    bool do_error;
    bool ec_error;
    bool nh4_error;
    unsigned long lastRead;
};

class SensorManager {
private:
    HardwareSerial SerialRS485;
    OneWire oneWire;
    DallasTemperature ds18b20;
    SensorData currentData;
    
    uint16_t calculateCRC(uint8_t *data, uint8_t length);
    bool sendModbusRequest(uint8_t slaveID, uint8_t functionCode, uint16_t startAddr, 
                          uint16_t numRegisters, uint8_t *response, uint8_t *respLen, 
                          uint32_t timeout = 1000);
    
    bool readDS18B20();
    bool readPHSensor();
    bool readDOSensor();
    bool readECTDS_Salinity_Sensor();
    bool readNH4_Sensor();
    
public:
    SensorManager();
    bool begin();
    bool readAllSensors();
    SensorData getSensorData();
    String getJSONPayload();
    bool discoverSensors();
    void resetErrors();
    bool calibrateSensor(uint8_t sensorType, float referenceValue);
    String getSensorStatus();
};

extern SensorManager sensorManager;

#endif