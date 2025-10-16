#include "calibration_manager.h"

CalibrationManager calibrationManager;

CalibrationManager::CalibrationManager() : 
    calibrating(false),
    currentCalibration(CALIB_NONE),
    calibStartTime(0),
    stabilityStartTime(0),
    initialValue(0),
    currentValue(0),
    valueStable(false),
    progress(0) {
}

void CalibrationManager::beginCalibration(CalibrationType type) {
    if (calibrating) return;
    
    currentCalibration = type;
    calibrating = true;
    calibStartTime = millis();
    stabilityStartTime = 0;
    valueStable = false;
    progress = 0;
    
    // Get initial value based on calibration type
    SensorData data = sensorManager.getSensorData();
    switch (type) {
        case CALIB_PH_4_01:
        case CALIB_PH_7_00:
        case CALIB_PH_10_01:
            initialValue = data.ph_value;
            break;
        case CALIB_DO_ZERO:
        case CALIB_DO_SLOPE:
            initialValue = data.do_value;
            break;
        case CALIB_EC_1413:
        case CALIB_EC_12880:
            initialValue = data.ec_value;
            break;
        default:
            initialValue = 0;
    }
    currentValue = initialValue;
    
    Serial.println("Calibration started: " + getSensorName(type));
}

void CalibrationManager::updateCalibration() {
    if (!calibrating) return;
    
    // Read current sensor value
    SensorData data = sensorManager.getSensorData();
    float newValue = 0;
    
    switch (currentCalibration) {
        case CALIB_PH_4_01:
        case CALIB_PH_7_00:
        case CALIB_PH_10_01:
            newValue = data.ph_value;
            break;
        case CALIB_DO_ZERO:
        case CALIB_DO_SLOPE:
            newValue = data.do_value;
            break;
        case CALIB_EC_1413:
        case CALIB_EC_12880:
            newValue = data.ec_value;
            break;
        default:
            newValue = 0;
    }
    
    currentValue = newValue;
    
    // Check stability
    if (checkStability(newValue)) {
        if (!valueStable) {
            valueStable = true;
            stabilityStartTime = millis();
        }
        
        // Calculate progress based on stable time (30 seconds total)
        unsigned long stableTime = millis() - stabilityStartTime;
        progress = map(min(stableTime, 30000UL), 0, 30000, 0, 100);
        
        // If stable for required time, complete calibration
        if (stableTime >= 30000) {
            sendCalibrationCommand();
            calibrating = false;
            Serial.println("Calibration completed successfully");
        }
    } else {
        valueStable = false;
        stabilityStartTime = 0;
        progress = 0;
    }
}

bool CalibrationManager::checkStability(float newValue) {
    // Check if value is within 1% of initial value for stability
    float tolerance = 0.01 * initialValue;
    return abs(newValue - initialValue) <= tolerance;
}

void CalibrationManager::sendCalibrationCommand() {
    // Send Modbus command to calibrate sensor
    // This is a placeholder - implement based on specific sensor protocols
    Serial.println("Sending calibration command for: " + getSensorName(currentCalibration));
    
    // Example for pH calibration (would need actual Modbus commands)
    switch (currentCalibration) {
        case CALIB_PH_4_01:
            // Send pH 4.01 calibration command
            break;
        case CALIB_PH_7_00:
            // Send pH 7.00 calibration command  
            break;
        case CALIB_PH_10_01:
            // Send pH 10.01 calibration command
            break;
        case CALIB_DO_ZERO:
            // Send DO zero calibration command
            break;
        case CALIB_DO_SLOPE:
            // Send DO slope calibration command
            break;
        case CALIB_EC_1413:
            // Send EC 1413 calibration command
            break;
        case CALIB_EC_12880:
            // Send EC 12880 calibration command
            break;
        default:
            break;
    }
}

void CalibrationManager::cancelCalibration() {
    calibrating = false;
    currentCalibration = CALIB_NONE;
    Serial.println("Calibration cancelled");
}

String CalibrationManager::getCalibrationInstruction(CalibrationType type) {
    switch (type) {
        case CALIB_PH_4_01:
            return "Immerse in pH 4.01 solution and wait for stabilization";
        case CALIB_PH_7_00:
            return "Immerse in pH 7.00 solution and wait for stabilization";
        case CALIB_PH_10_01:
            return "Immerse in pH 10.01 solution and wait for stabilization";
        case CALIB_DO_ZERO:
            return "Immerse in zero oxygen solution and wait for stabilization";
        case CALIB_DO_SLOPE:
            return "Immerse in air-saturated water and wait for stabilization";
        case CALIB_EC_1413:
            return "Immerse in 1413 µS/cm solution and wait for stabilization";
        case CALIB_EC_12880:
            return "Immerse in 12880 µS/cm solution and wait for stabilization";
        default:
            return "Unknown calibration type";
    }
}

String CalibrationManager::getSensorName(CalibrationType type) {
    switch (type) {
        case CALIB_PH_4_01: return "pH 4.01";
        case CALIB_PH_7_00: return "pH 7.00";
        case CALIB_PH_10_01: return "pH 10.01";
        case CALIB_DO_ZERO: return "DO Zero";
        case CALIB_DO_SLOPE: return "DO Slope";
        case CALIB_EC_1413: return "EC 1413µS";
        case CALIB_EC_12880: return "EC 12880µS";
        default: return "Unknown";
    }
}

// Getters
bool CalibrationManager::isCalibrating() { return calibrating; }
CalibrationType CalibrationManager::getCurrentCalibration() { return currentCalibration; }
String CalibrationManager::getInstruction() { return getCalibrationInstruction(currentCalibration); }
float CalibrationManager::getCurrentValue() { return currentValue; }
int CalibrationManager::getProgress() { return progress; }
bool CalibrationManager::isStable() { return valueStable; }