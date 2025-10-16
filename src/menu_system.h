#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include <Encoder.h>
#include "config.h"
#include "display_manager.h"
#include "sensor_manager.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "calibration_manager.h"

enum MenuState {
    MENU_MAIN,
    MENU_SENSOR_DISPLAY,
    MENU_SENSOR_DETAIL,
    MENU_WIFI_CONFIG,
    MENU_CALIBRATION,
    MENU_CALIBRATION_PROGRESS,
    MENU_TIME_CONFIG,
    MENU_SYSTEM_INFO,
    MENU_SETTINGS,
    MENU_MESSAGE
};

class MenuSystem {
private:
    Encoder encoder;
    MenuState currentState;
    MenuState previousState;
    
    // Navigation variables
    long encoderPosition;
    long lastEncoderPosition;
    int selectedItem;
    int scrollOffset;
    
    // Button states
    bool buttonAState;
    bool buttonBState;
    bool encoderButtonState;
    bool lastButtonAState;
    bool lastButtonBState;
    bool lastEncoderButtonState;
    unsigned long lastButtonPress;
    
    // Menu content
    String mainMenu[8] = {
        "📊 Sensor Overview",
        "🔍 Sensor Details", 
        "📶 WiFi Settings",
        "⚙️ Calibration",
        "🕐 Time Settings",
        "ℹ️ System Info",
        "⚡ Settings",
        "🔃 Refresh Data"
    };
    
    String sensorDetailMenu[6] = {
        "🌡️ Temperature",
        "🧪 pH Value", 
        "💧 Dissolved Oxygen",
        "⚡ Conductivity",
        "🔬 Ammonia",
        "🧂 Salinity"
    };
    
    String calibrationMenu[7] = {
        "🧪 pH 4.01 Point",
        "🧪 pH 7.00 Point", 
        "🧪 pH 10.01 Point",
        "💧 DO Zero Point",
        "💧 DO Slope Point", 
        "⚡ EC 1413µS/cm",
        "⚡ EC 12880µS/cm"
    };
    
    String settingsMenu[4] = {
        "🔧 Factory Reset",
        "🔄 Reboot System",
        "📊 Data Interval",
        "🔒 Device Lock"
    };
    
    // Input handling
    void handleEncoder();
    void handleButtons();
    void handleButtonAction(int button);
    
    // Navigation
    void navigateTo(MenuState newState);
    void goBack();
    void executeMenuItem();
    
    // State handlers
    void handleMainMenu();
    void handleSensorDisplay();
    void handleSensorDetail();
    void handleWiFiConfig();
    void handleCalibration();
    void handleCalibrationProgress();
    void handleTimeConfig();
    void handleSystemInfo();
    void handleSettings();
    void handleMessage();
    
public:
    MenuSystem();
    void begin();
    void update();
    MenuState getCurrentState();
    void showMessage(const String& title, const String& message, bool success = true);
};

extern MenuSystem menuSystem;

#endif