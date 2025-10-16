#include "sensor_manager.h"
#include "time_manager.h"

SensorManager sensorManager;

SensorManager::SensorManager() : 
    SerialRS485(1), 
    oneWire(ONE_WIRE_BUS), 
    ds18b20(&oneWire) {
    
    // Initialize sensor data structure
    memset(&currentData, 0, sizeof(currentData));
    currentData.timestamp = "2024-01-01 00:00:00";
    currentData.lastRead = 0;
}

bool SensorManager::begin() {
    Utils::info("Initializing Sensor Manager...");
    
    // Initialize RS485
    SerialRS485.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    pinMode(RS485_DE_RE_PIN, OUTPUT);
    digitalWrite(RS485_DE_RE_PIN, LOW);
    
    // Initialize DS18B20
    ds18b20.begin();
    
    // Test sensor communication
    bool sensorsOK = discoverSensors();
    
    Utils::info(sensorsOK ? "✅ Sensors initialized successfully" : "❌ Sensor initialization failed");
    return sensorsOK;
}

bool SensorManager::readAllSensors() {
    bool allSuccess = true;
    
    Utils::debug("Reading all sensors...");
    
    allSuccess &= readDS18B20();
    allSuccess &= readPHSensor();
    allSuccess &= readDOSensor();
    allSuccess &= readECTDS_Salinity_Sensor();
    allSuccess &= readNH4_Sensor();
    
    // Update timestamp
    currentData.timestamp = timeManager.getCurrentTimestamp();
    currentData.lastRead = millis();
    
    Utils::debug("Sensor read completed: " + String(allSuccess ? "SUCCESS" : "SOME ERRORS"));
    return allSuccess;
}

bool SensorManager::readDS18B20() {
    ds18b20.requestTemperatures();
    float temp = ds18b20.getTempCByIndex(0);
    
    if (temp != DEVICE_DISCONNECTED_C && temp > -50 && temp < 150) {
        currentData.ds18b20_temp = temp;
        currentData.ds18b20_error = false;
        Utils::debug("DS18B20: " + String(temp, 2) + "°C");
        return true;
    } else {
        currentData.ds18b20_error = true;
        Utils::error("DS18B20 read error");
        return false;
    }
}

bool SensorManager::readPHSensor() {
    uint8_t response[32];
    uint8_t respLen = 0;
    
    if (sendModbusRequest(PH_SENSOR_ID, 0x04, 0x0001, 0x0001, response, &respLen, 500)) {
        if (respLen >= 5 && response[0] == PH_SENSOR_ID && response[1] == 0x04) {
            uint16_t phRaw = (response[3] << 8) | response[4];
            currentData.ph_value = phRaw / 100.0;
            currentData.ph_error = false;
            Utils::debug("pH: " + String(currentData.ph_value, 2));
            return true;
        }
    }
    
    currentData.ph_error = true;
    Utils::error("pH Sensor read error");
    return false;
}

bool SensorManager::readDOSensor() {
    uint8_t response[32];
    uint8_t respLen = 0;
    
    if (sendModbusRequest(DO_SENSOR_ID, 0x03, 0x0101, 0x0001, response, &respLen, 500)) {
        if (respLen >= 5 && response[0] == DO_SENSOR_ID && response[1] == 0x03) {
            uint16_t doRaw = (response[3] << 8) | response[4];
            currentData.do_value = doRaw / 100.0;
            currentData.do_error = false;
            Utils::debug("DO: " + String(currentData.do_value, 2) + " mg/L");
            return true;
        }
    }
    
    currentData.do_error = true;
    Utils::error("DO Sensor read error");
    return false;
}

bool SensorManager::readECTDS_Salinity_Sensor() {
    uint8_t response[32];
    uint8_t respLen = 0;
    bool success = true;
    
    // Read EC value
    if (sendModbusRequest(EC_SENSOR_ID, 0x04, 0x0002, 0x0001, response, &respLen, 500)) {
        if (respLen >= 5 && response[0] == EC_SENSOR_ID && response[1] == 0x04) {
            currentData.ec_value = (response[3] << 8) | response[4];
            currentData.ec_error = false;
            Utils::debug("EC: " + String(currentData.ec_value) + " uS/cm");
        }
    } else {
        currentData.ec_error = true;
        success = false;
    }
    
    // Read TDS value
    respLen = 0;
    if (sendModbusRequest(EC_SENSOR_ID, 0x04, 0x0004, 0x0001, response, &respLen, 500)) {
        if (respLen >= 5 && response[0] == EC_SENSOR_ID && response[1] == 0x04) {
            currentData.tds_value = (response[3] << 8) | response[4];
            Utils::debug("TDS: " + String(currentData.tds_value) + " ppm");
        }
    }
    
    // Read Salinity value
    respLen = 0;
    if (sendModbusRequest(EC_SENSOR_ID, 0x04, 0x0003, 0x0001, response, &respLen, 500)) {
        if (respLen >= 5 && response[0] == EC_SENSOR_ID && response[1] == 0x04) {
            currentData.salinitas_value = (response[3] << 8) | response[4];
            Utils::debug("Salinity: " + String(currentData.salinitas_value) + " mg/L");
        }
    }
    
    if (!success) {
        Utils::error("EC/TDS Sensor read error");
    }
    
    return success;
}

bool SensorManager::readNH4_Sensor() {
    uint8_t response[32];
    uint8_t respLen = 0;
    
    if (sendModbusRequest(NH4_SENSOR_ID, 0x03, 0x0000, 0x0002, response, &respLen, 500)) {
        if (respLen >= 9 && response[0] == NH4_SENSOR_ID && response[1] == 0x03) {
            uint32_t rawValue = (response[3] << 24) | (response[4] << 16) | (response[5] << 8) | response[6];
            memcpy(&currentData.ammonia_value, &rawValue, sizeof(currentData.ammonia_value));
            currentData.nh4_error = false;
            Utils::debug("NH4: " + String(currentData.ammonia_value, 4) + " ppm");
            return true;
        }
    }
    
    currentData.nh4_error = true;
    Utils::error("NH4 Sensor read error");
    return false;
}

bool SensorManager::discoverSensors() {
    Utils::info("Discovering sensors...");
    bool foundAny = false;
    
    int sensorIDs[] = {PH_SENSOR_ID, DO_SENSOR_ID, EC_SENSOR_ID, NH4_SENSOR_ID};
    const char* sensorNames[] = {"pH", "DO", "EC/TDS", "NH4"};
    
    for (int i = 0; i < 4; i++) {
        uint8_t response[32];
        uint8_t respLen = 0;
        
        if (sendModbusRequest(sensorIDs[i], 0x04, 0x0000, 0x0001, response, &respLen, 300)) {
            Utils::info("✅ Found " + String(sensorNames[i]) + " sensor at ID: " + String(sensorIDs[i]));
            foundAny = true;
        } else {
            Utils::error("❌ " + String(sensorNames[i]) + " sensor not found at ID: " + String(sensorIDs[i]));
        }
        delay(100);
    }
    
    // Test DS18B20
    ds18b20.requestTemperatures();
    if (ds18b20.getTempCByIndex(0) != DEVICE_DISCONNECTED_C) {
        Utils::info("✅ Found DS18B20 temperature sensor");
        foundAny = true;
    } else {
        Utils::error("❌ DS18B20 not found");
    }
    
    return foundAny;
}

uint16_t SensorManager::calculateCRC(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

bool SensorManager::sendModbusRequest(uint8_t slaveID, uint8_t functionCode, uint16_t startAddr, 
                                     uint16_t numRegisters, uint8_t *response, uint8_t *respLen, 
                                     uint32_t timeout) {
    // Build request
    uint8_t request[8];
    request[0] = slaveID;
    request[1] = functionCode;
    request[2] = (startAddr >> 8) & 0xFF;
    request[3] = startAddr & 0xFF;
    request[4] = (numRegisters >> 8) & 0xFF;
    request[5] = numRegisters & 0xFF;
    
    // Calculate CRC
    uint16_t crc = calculateCRC(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    // Clear buffer
    while (SerialRS485.available()) {
        SerialRS485.read();
    }
    
    // Send request
    digitalWrite(RS485_DE_RE_PIN, HIGH);
    delay(2);
    SerialRS485.write(request, 8);
    SerialRS485.flush();
    delay(2);
    digitalWrite(RS485_DE_RE_PIN, LOW);
    
    // Read response
    uint32_t startTime = millis();
    uint8_t index = 0;
    
    while ((millis() - startTime) < timeout) {
        while (SerialRS485.available() && index < 32) {
            response[index++] = SerialRS485.read();
        }
        
        if (index >= 5) {
            uint8_t expectedLength = 5 + response[2];
            if (index >= expectedLength) {
                *respLen = index;
                
                // Verify CRC
                uint16_t receivedCRC = (response[expectedLength-1] << 8) | response[expectedLength-2];
                uint16_t calculatedCRC = calculateCRC(response, expectedLength-2);
                
                if (receivedCRC == calculatedCRC) {
                    Utils::debug("Modbus request successful");
                    return true;
                } else {
                    Utils::error("CRC Error in Modbus response");
                    return false;
                }
            }
        }
        delay(10);
    }
    
    *respLen = index;
    Utils::error("Modbus request timeout");
    return false;
}

SensorData SensorManager::getSensorData() {
    return currentData;
}

String SensorManager::getJSONPayload() {
    DynamicJsonDocument doc(512);
    doc["uid"] = DEVICE_UID;
    doc["suhu"] = round(currentData.ds18b20_temp * 100.0) / 100.0;
    doc["ph"] = round(currentData.ph_value * 100.0) / 100.0;
    doc["do"] = round(currentData.do_value * 100.0) / 100.0;
    doc["tds"] = round(currentData.tds_value * 10.0) / 10.0;
    doc["ammonia"] = round(currentData.ammonia_value * 1000.0) / 1000.0;
    doc["salinitas"] = round(currentData.salinitas_value * 100.0) / 100.0;
    doc["timestamp"] = currentData.timestamp;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void SensorManager::resetErrors() {
    currentData.ds18b20_error = false;
    currentData.ph_error = false;
    currentData.do_error = false;
    currentData.ec_error = false;
    currentData.nh4_error = false;
}

bool SensorManager::calibrateSensor(uint8_t sensorType, float referenceValue) {
    // Implementation for sensor calibration
    Utils::info("Calibrating sensor type: " + String(sensorType));
    return true;
}

String SensorManager::getSensorStatus() {
    int workingSensors = 5;
    if (currentData.ds18b20_error) workingSensors--;
    if (currentData.ph_error) workingSensors--;
    if (currentData.do_error) workingSensors--;
    if (currentData.ec_error) workingSensors--;
    if (currentData.nh4_error) workingSensors--;
    
    return String(workingSensors) + "/5 OK";
}