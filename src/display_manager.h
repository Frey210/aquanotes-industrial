#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "config.h"
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "time_manager.h"

class DisplayManager {
private:
    Adafruit_SH1106G display;
    bool displayAvailable;
    unsigned long lastUpdate;
    
    void drawHeader(const String& title, bool showStatus = true);
    void drawFooter(const String& line1, const String& line2 = "");
    void drawProgressBar(int x, int y, int width, int height, int progress);
    
public:
    DisplayManager();
    bool begin();
    void clear();
    void update();
    
    // Main display functions
    void showSplashScreen();
    void showSensorData(const SensorData& data);
    void showMainMenu(const String menuItems[], int itemCount, int selectedIndex, int scrollOffset);
    void showSensorDetail(const SensorData& data, int selectedSensor);
    void showWiFiConfig(const String& ssid, const String& status, const String& ip, int rssi, bool apMode);
    void showCalibrationMenu(const String menuItems[], int itemCount, int selectedIndex, int scrollOffset);
    void showCalibrationProgress(const String& sensorName, const String& instruction, float currentValue, int progress);
    void showTimeConfig(int timezone, const String& currentTime, bool timeSynced);
    void showSystemInfo(const SensorData& data, const String& uptime, int workingSensors);
    void showSettingsMenu(const String menuItems[], int itemCount, int selectedIndex);
    void showMessage(const String& title, const String& message, bool success = true);
    void showError(const String& title, const String& message);
    void showScanningAnimation(const String& message);
    
    // Utility functions
    void setCursor(int x, int y);
    void print(const String& text);
    void println(const String& text);
    void drawHorizontalLine(int y);
    void drawVerticalLine(int x);
    void drawBatteryIcon(int x, int y, int level); // 0-100%
    void drawWiFiIcon(int x, int y, bool connected, int rssi);
    void drawClockIcon(int x, int y, bool synced);
};

extern DisplayManager displayManager;

#endif