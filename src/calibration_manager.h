#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"

enum CalibrationType {
    CALIB_NONE = 0,
    CALIB_PH_4_01 = 1,
    CALIB_PH_7_00 = 2,
    CALIB_PH_10_01 = 3,
    CALIB_DO_ZERO = 4,
    CALIB_DO_SLOPE = 5,
    CALIB_EC_1413 = 6,
    CALIB_EC_12880 = 7
};

class CalibrationManager {
private:
    bool calibrating;
    CalibrationType currentCalibration;
    unsigned long calibStartTime;
    unsigned long stabilityStartTime;
    float initialValue;
    float currentValue;
    bool valueStable;
    int progress;
    
    String getCalibrationInstruction(CalibrationType type);
    String getSensorName(CalibrationType type);
    bool checkStability(float newValue);
    void sendCalibrationCommand();
    
public:
    CalibrationManager();
    void beginCalibration(CalibrationType type);
    void updateCalibration();
    void cancelCalibration();
    bool isCalibrating();
    CalibrationType getCurrentCalibration();
    String getInstruction();
    float getCurrentValue();
    int getProgress();
    bool isStable();
};

extern CalibrationManager calibrationManager;

#endif