#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include <Preferences.h>
#include "config.h"
#include "utils.h"

class TimeManager {
private:
    Preferences preferences;
    int timezone;
    bool timeSynced;
    unsigned long lastSyncAttempt;
    
    bool syncWithNTP();
    
public:
    TimeManager();
    bool begin();
    String getCurrentTimestamp();
    String getFormattedTime();
    void setTimezone(int newTimezone);
    int getTimezone();
    bool syncTime();
    bool isTimeSynced();
    String getUptime();
    void forceResync();
};

extern TimeManager timeManager;

#endif