#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "config.h"
#include "utils.h"

class WiFiManager {
private:
    Preferences preferences;
    String ssid;
    String password;
    String serverURL;
    bool wifiConnected;
    bool apMode;
    unsigned long lastReconnectAttempt;
    unsigned long lastStatusCheck;
    
    bool connectToWiFi();
    void startAPMode();
    
public:
    WiFiManager();
    bool begin();
    bool isConnected();
    String getSSID();
    String getIPAddress();
    int getRSSI();
    void setCredentials(const String& newSSID, const String& newPassword);
    bool testConnection();
    bool sendDataToServer(const String& jsonPayload);
    void handleReconnection();
    void resetSettings();
    void startConfigurationMode();
    bool isAPMode();
    void scanNetworks();
};

extern WiFiManager wifiManager;

#endif