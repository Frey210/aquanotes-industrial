#include "time_manager.h"

TimeManager timeManager;

TimeManager::TimeManager() : 
    timezone(DEFAULT_TIMEZONE), 
    timeSynced(false),
    lastSyncAttempt(0) {
}

bool TimeManager::begin() {
    preferences.begin("time-config", false);
    
    // Load saved timezone
    timezone = preferences.getInt("timezone", DEFAULT_TIMEZONE);
    
    Utils::info("Time Manager Started - Timezone: GMT+" + String(timezone));
    
    // Initial time sync
    return syncTime();
}

bool TimeManager::syncTime() {
    if (millis() - lastSyncAttempt < 60000) { // Don't sync more than once per minute
        return timeSynced;
    }
    
    lastSyncAttempt = millis();
    
    Utils::info("Syncing time with NTP server...");
    
    configTime(timezone * 3600, 0, NTP_SERVER);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, NTP_TIMEOUT)) {
        Utils::error("Failed to obtain time from NTP");
        timeSynced = false;
        return false;
    }
    
    timeSynced = true;
    Utils::info("Time synchronized: " + getFormattedTime());
    return true;
}

String TimeManager::getCurrentTimestamp() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "2024-01-01 00:00:00";
    }
    
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timestamp);
}

String TimeManager::getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time Sync Failed";
    }
    
    char timeString[25];
    strftime(timeString, sizeof(timeString), "%a %b %d %H:%M:%S %Y", &timeinfo);
    return String(timeString);
}

void TimeManager::setTimezone(int newTimezone) {
    if (newTimezone >= -12 && newTimezone <= 14) {
        timezone = newTimezone;
        preferences.putInt("timezone", timezone);
        Utils::info("Timezone updated to GMT+" + String(timezone));
        
        // Resync time with new timezone
        syncTime();
    }
}

int TimeManager::getTimezone() {
    return timezone;
}

bool TimeManager::isTimeSynced() {
    return timeSynced;
}

String TimeManager::getUptime() {
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    char uptime[20];
    snprintf(uptime, sizeof(uptime), "%lud %02lu:%02lu:%02lu", 
             days, hours % 24, minutes % 60, seconds % 60);
    return String(uptime);
}

void TimeManager::forceResync() {
    timeSynced = false;
    syncTime();
}