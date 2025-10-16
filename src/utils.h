#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "config.h"

class Utils {
public:
    static void debug(const String& message) {
        if (DEBUG_MODE) {
            Serial.println("[DEBUG] " + message);
        }
    }
    
    static void error(const String& message, ErrorCode code = ERROR_NONE) {
        Serial.println("[ERROR] " + message + " Code: " + String(code));
    }
    
    static void info(const String& message) {
        Serial.println("[INFO] " + message);
    }
    
    static String formatFloat(float value, int decimals = 2) {
        char buffer[20];
        dtostrf(value, 1, decimals, buffer);
        return String(buffer);
    }
    
    static String getDeviceUID() {
        return String(DEVICE_UID);
    }
    
    static String getChipID() {
        uint64_t chipid = ESP.getEfuseMac();
        uint32_t high = (chipid >> 32) & 0xFFFFFFFF;
        uint32_t low = chipid & 0xFFFFFFFF;
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%08X%08X", high, low);
        return String(buffer);
    }
    
    static bool isNumber(const String& str) {
        for (unsigned int i = 0; i < str.length(); i++) {
            if (!isDigit(str.charAt(i)) && str.charAt(i) != '.' && str.charAt(i) != '-') {
                return false;
            }
        }
        return true;
    }
    
    static String bytesToHex(const uint8_t* data, size_t length) {
        String result = "";
        for (size_t i = 0; i < length; i++) {
            if (data[i] < 16) result += "0";
            result += String(data[i], HEX);
            result += " ";
        }
        return result;
    }
};

#endif