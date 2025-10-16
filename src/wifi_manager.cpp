#include "wifi_manager.h"

WiFiManager wifiManager;

WiFiManager::WiFiManager() : 
    wifiConnected(false), 
    apMode(false),
    lastReconnectAttempt(0),
    lastStatusCheck(0) {
}

bool WiFiManager::begin() {
    preferences.begin("wifi-config", false);
    
    // Load saved credentials
    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");
    serverURL = preferences.getString("serverURL", API_URL);
    
    Utils::info("WiFi Manager Started");
    Utils::info("Saved SSID: " + String(ssid.length() > 0 ? ssid : "None"));
    
    if (ssid.length() > 0) {
        return connectToWiFi();
    } else {
        Utils::info("No WiFi credentials saved. Starting AP mode.");
        startAPMode();
        return false;
    }
}

bool WiFiManager::connectToWiFi() {
    if (ssid.length() == 0) {
        Utils::error("No SSID configured");
        return false;
    }
    
    Utils::info("Connecting to: " + ssid);
    
    // Stop AP mode if active
    if (apMode) {
        WiFi.softAPdisconnect(true);
        apMode = false;
    }
    
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        if (attempts % 10 == 0) {
            Serial.println();
        }
        
        if (attempts >= 30) {
            break;
        }
    }
    
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    
    if (wifiConnected) {
        Utils::info("✅ WiFi Connected!");
        Utils::info("IP: " + WiFi.localIP().toString());
        Utils::info("RSSI: " + String(WiFi.RSSI()) + " dBm");
    } else {
        Utils::error("❌ WiFi Connection Failed!");
        Utils::error("Status: " + String(WiFi.status()));
        
        // Fallback to AP mode
        startAPMode();
    }
    
    lastReconnectAttempt = millis();
    return wifiConnected;
}

void WiFiManager::startAPMode() {
    WiFi.disconnect();
    delay(100);
    
    String apSSID = "WaterSensor-" + String(DEVICE_UID);
    bool result = WiFi.softAP(apSSID.c_str());
    
    apMode = true;
    wifiConnected = false;
    
    if (result) {
        Utils::info("AP Mode Started");
        Utils::info("SSID: " + apSSID);
        Utils::info("IP: " + WiFi.softAPIP().toString());
    } else {
        Utils::error("Failed to start AP mode");
    }
}

bool WiFiManager::sendDataToServer(const String& jsonPayload) {
    if (!wifiConnected) {
        Utils::error("WiFi not connected, cannot send data");
        return false;
    }
    
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(API_TIMEOUT);
    
    Utils::debug("Sending data to: " + serverURL);
    Utils::debug("Payload: " + jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    String response = http.getString();
    
    bool success = (httpResponseCode == 200);
    
    if (success) {
        Utils::info("✅ Data sent successfully");
        Utils::debug("Response: " + response);
    } else {
        Utils::error("❌ HTTP Error: " + String(httpResponseCode));
        Utils::error("Error: " + http.errorToString(httpResponseCode));
        Utils::debug("Response: " + response);
    }
    
    http.end();
    return success;
}

void WiFiManager::setCredentials(const String& newSSID, const String& newPassword) {
    ssid = newSSID;
    password = newPassword;
    
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    
    Utils::info("WiFi credentials updated");
    Utils::info("SSID: " + ssid);
    Utils::info("Password: " + String(password.length() > 0 ? "***" : "None"));
    
    // Reconnect with new credentials
    connectToWiFi();
}

void WiFiManager::handleReconnection() {
    if (!wifiConnected && millis() - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL) {
        Utils::info("Attempting WiFi reconnection...");
        connectToWiFi();
    }
    
    // Periodic status check
    if (wifiConnected && millis() - lastStatusCheck > 10000) {
        if (WiFi.status() != WL_CONNECTED) {
            Utils::info("WiFi connection lost");
            wifiConnected = false;
        }
        lastStatusCheck = millis();
    }
}

bool WiFiManager::testConnection() {
    if (!wifiConnected) return false;
    
    HTTPClient http;
    http.begin("http://www.google.com");
    http.setTimeout(5000);
    int responseCode = http.GET();
    http.end();
    
    bool connected = (responseCode == 200);
    Utils::info("Internet connection test: " + String(connected ? "OK" : "FAILED"));
    return connected;
}

void WiFiManager::resetSettings() {
    preferences.remove("ssid");
    preferences.remove("password");
    ssid = "";
    password = "";
    
    Utils::info("WiFi settings reset");
    startAPMode();
}

void WiFiManager::scanNetworks() {
    if (wifiConnected) return;
    
    Utils::info("Scanning WiFi networks...");
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Utils::info("No networks found");
    } else {
        Utils::info(String(n) + " networks found:");
        for (int i = 0; i < n; ++i) {
            Utils::info(String(i + 1) + ": " + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)");
        }
    }
}

// Getters
bool WiFiManager::isConnected() { return wifiConnected && (WiFi.status() == WL_CONNECTED); }
String WiFiManager::getSSID() { return ssid; }
String WiFiManager::getIPAddress() { 
    return wifiConnected ? WiFi.localIP().toString() : (apMode ? WiFi.softAPIP().toString() : "N/A"); 
}
int WiFiManager::getRSSI() { return wifiConnected ? WiFi.RSSI() : 0; }
bool WiFiManager::isAPMode() { return apMode; }