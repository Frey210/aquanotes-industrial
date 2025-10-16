#include "display_manager.h"

DisplayManager displayManager;

DisplayManager::DisplayManager() : 
    display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST),
    displayAvailable(false),
    lastUpdate(0) {
}

bool DisplayManager::begin() {
    Wire.begin(OLED_SDA, OLED_SCL);
    
    if (!display.begin(OLED_ADDRESS, true)) {
        Serial.println("❌ SH1106 display not found");
        return false;
    }
    
    displayAvailable = true;
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    Serial.println("✅ OLED Display initialized");
    return true;
}

void DisplayManager::clear() {
    if (!displayAvailable) return;
    display.clearDisplay();
    display.setCursor(0, 0);
}

void DisplayManager::update() {
    if (!displayAvailable) return;
    display.display();
}

void DisplayManager::drawHeader(const String& title, bool showStatus) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(title);
    
    if (showStatus) {
        // WiFi status
        display.setCursor(90, 0);
        drawWiFiIcon(90, 0, wifiManager.isConnected(), wifiManager.getRSSI());
        
        // Time status
        display.setCursor(110, 0);
        drawClockIcon(110, 0, timeManager.isTimeSynced());
    }
    
    drawHorizontalLine(9);
}

void DisplayManager::drawFooter(const String& line1, const String& line2) {
    drawHorizontalLine(54);
    display.setCursor(2, 56);
    display.print(line1);
    
    if (line2.length() > 0) {
        display.setCursor(2, 58);
        display.print(line2);
    }
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, int progress) {
    // Draw border
    display.drawRect(x, y, width, height, SH110X_WHITE);
    
    // Draw progress
    int fillWidth = map(progress, 0, 100, 0, width - 2);
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SH110X_WHITE);
}

void DisplayManager::showSplashScreen() {
    if (!displayAvailable) return;
    
    clear();
    display.setTextSize(2);
    display.setCursor(10, 5);
    display.println("WATER");
    display.setCursor(25, 25);
    display.println("QUALITY");
    display.setTextSize(1);
    display.setCursor(35, 50);
    display.println("MONITOR");
    update();
}

void DisplayManager::showSensorData(const SensorData& data) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("📊 SENSOR DATA", true);
    
    // Sensor values in compact grid
    display.setCursor(0, 12);
    display.print("🌡 ");
    display.print(data.ds18b20_temp, 1);
    display.print("C");
    
    display.setCursor(64, 12);
    display.print("pH ");
    display.print(data.ph_value, 1);
    
    display.setCursor(0, 22);
    display.print("💧 ");
    display.print(data.do_value, 1);
    display.print("mg");
    
    display.setCursor(64, 22);
    display.print("⚡ ");
    display.print(data.ec_value);
    display.print("uS");
    
    display.setCursor(0, 32);
    display.print("🔬 ");
    display.print(data.ammonia_value, 2);
    display.print("ppm");
    
    display.setCursor(64, 32);
    display.print("🧂 ");
    display.print(data.salinitas_value, 0);
    
    display.setCursor(0, 42);
    display.print("💎 TDS:");
    display.print(data.tds_value);
    display.print("ppm");
    
    // Error indicator
    if (data.ds18b20_error || data.ph_error || data.do_error || data.ec_error || data.nh4_error) {
        display.setCursor(120, 0);
        display.print("⚠");
    }
    
    drawFooter("A:Refresh B:Menu", data.timestamp.substring(11, 16));
    update();
}

void DisplayManager::showMainMenu(const String menuItems[], int itemCount, int selectedIndex, int scrollOffset) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("🌊 MAIN MENU", false);
    
    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        int itemIndex = i + scrollOffset;
        if (itemIndex >= itemCount) break;
        
        display.setCursor(2, 12 + i * 12);
        
        if (itemIndex == selectedIndex) {
            display.print(">");
        } else {
            display.print(" ");
        }
        
        display.println(menuItems[itemIndex]);
    }
    
    // Scroll indicators
    if (scrollOffset > 0) {
        display.setCursor(122, 12);
        display.print("↑");
    }
    if (scrollOffset + MAX_VISIBLE_ITEMS < itemCount) {
        display.setCursor(122, 54);
        display.print("↓");
    }
    
    drawFooter("A:Select B:Back", "Enc:Navigate");
    update();
}

void DisplayManager::showSensorDetail(const SensorData& data, int selectedSensor) {
    if (!displayAvailable) return;
    
    clear();
    
    String sensorNames[] = {"TEMPERATURE", "pH VALUE", "DISSOLVED OXYGEN", "CONDUCTIVITY", "AMMONIA", "SALINITY"};
    String sensorIcons[] = {"🌡", "🧪", "💧", "⚡", "🔬", "🧂"};
    
    display.setCursor(0, 0);
    display.print(sensorIcons[selectedSensor]);
    display.print(" ");
    display.println(sensorNames[selectedSensor]);
    drawHorizontalLine(9);
    
    display.setCursor(0, 12);
    
    switch (selectedSensor) {
        case 0: // Temperature
            display.println("Sensor: DS18B20");
            display.print("Value: ");
            display.print(data.ds18b20_temp, 2);
            display.println(" °C");
            display.print("Status: ");
            display.println(data.ds18b20_error ? "❌ ERROR" : "✅ OK");
            display.print("Range: -55 to 125°C");
            break;
            
        case 1: // pH
            display.println("Sensor: pH Electrode");
            display.print("Value: ");
            display.print(data.ph_value, 3);
            display.println(" pH");
            display.print("Status: ");
            display.println(data.ph_error ? "❌ ERROR" : "✅ OK");
            display.print("Range: 0-14 pH");
            break;
            
        case 2: // DO
            display.println("Sensor: Optical DO");
            display.print("Value: ");
            display.print(data.do_value, 3);
            display.println(" mg/L");
            display.print("Status: ");
            display.println(data.do_error ? "❌ ERROR" : "✅ OK");
            display.print("Range: 0-20 mg/L");
            break;
            
        case 3: // Conductivity
            display.println("Sensor: EC Electrode");
            display.print("Value: ");
            display.print(data.ec_value);
            display.println(" µS/cm");
            display.print("TDS: ");
            display.print(data.tds_value);
            display.println(" ppm");
            display.print("Status: ");
            display.println(data.ec_error ? "❌ ERROR" : "✅ OK");
            break;
            
        case 4: // Ammonia
            display.println("Sensor: NH4 ISE");
            display.print("Value: ");
            display.print(data.ammonia_value, 4);
            display.println(" ppm");
            display.print("Status: ");
            display.println(data.nh4_error ? "❌ ERROR" : "✅ OK");
            break;
            
        case 5: // Salinity
            display.println("Derived from EC");
            display.print("Value: ");
            display.print(data.salinitas_value, 1);
            display.println(" PSU");
            display.print("Formula: EC × 0.5");
            break;
    }
    
    drawFooter("←→:Navigate B:Back", "A:Refresh Value");
    update();
}

void DisplayManager::showWiFiConfig(const String& ssid, const String& status, const String& ip, int rssi, bool apMode) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("📶 WIFI CONFIG", false);
    
    display.setCursor(0, 12);
    display.print("SSID: ");
    display.println(ssid.length() > 0 ? ssid : "Not Set");
    
    display.print("Status: ");
    display.println(status);
    
    display.print("IP: ");
    display.println(ip);
    
    if (!apMode && rssi != 0) {
        display.print("RSSI: ");
        display.print(rssi);
        display.println(" dBm");
    }
    
    display.print("Mode: ");
    display.println(apMode ? "Access Point" : "Client");
    
    drawFooter("A:Scan B:Back", "Enc:Change SSID");
    update();
}

void DisplayManager::showCalibrationMenu(const String menuItems[], int itemCount, int selectedIndex, int scrollOffset) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("⚙️ CALIBRATION", false);
    
    for (int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        int itemIndex = i + scrollOffset;
        if (itemIndex >= itemCount) break;
        
        display.setCursor(2, 12 + i * 12);
        
        if (itemIndex == selectedIndex) {
            display.print(">");
        } else {
            display.print(" ");
        }
        
        display.println(menuItems[itemIndex]);
    }
    
    drawFooter("A:Start Calib B:Back", "Enc:Select Point");
    update();
}

void DisplayManager::showCalibrationProgress(const String& sensorName, const String& instruction, float currentValue, int progress) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("🔧 CALIBRATING", false);
    
    display.setCursor(0, 12);
    display.print("Sensor: ");
    display.println(sensorName);
    
    display.setCursor(0, 22);
    display.println(instruction);
    
    display.setCursor(0, 32);
    display.print("Value: ");
    display.print(currentValue, 3);
    
    // Progress bar
    drawProgressBar(10, 42, 108, 8, progress);
    
    display.setCursor(50, 52);
    display.print(progress);
    display.print("%");
    
    update();
}

void DisplayManager::showTimeConfig(int timezone, const String& currentTime, bool timeSynced) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("🕐 TIME CONFIG", false);
    
    display.setCursor(0, 12);
    display.print("Timezone: GMT+");
    display.println(timezone);
    
    display.setCursor(0, 22);
    display.print("Current: ");
    display.println(currentTime);
    
    display.setCursor(0, 32);
    display.print("NTP Sync: ");
    display.println(timeSynced ? "✅ OK" : "❌ Failed");
    
    display.setCursor(0, 42);
    display.println("Rotate: Change TZ");
    
    drawFooter("A:Save B:Back", "Hold A:Resync");
    update();
}

void DisplayManager::showSystemInfo(const SensorData& data, const String& uptime, int workingSensors) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("ℹ️ SYSTEM INFO", false);
    
    display.setCursor(0, 12);
    display.print("UID: ");
    display.println(DEVICE_UID);
    
    display.print("Uptime: ");
    display.println(uptime);
    
    display.print("WiFi: ");
    display.println(wifiManager.isConnected() ? "Connected" : "Offline");
    
    display.print("Sensors: ");
    display.print(workingSensors);
    display.println("/6 OK");
    
    display.print("Last Read: ");
    unsigned long secondsAgo = (millis() - data.lastRead) / 1000;
    display.print(secondsAgo);
    display.println("s ago");
    
    display.print("Memory: ");
    display.print(ESP.getFreeHeap() / 1024);
    display.println(" KB free");
    
    drawFooter("B:Back to Menu", "");
    update();
}

void DisplayManager::showSettingsMenu(const String menuItems[], int itemCount, int selectedIndex) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("⚡ SETTINGS", false);
    
    for (int i = 0; i < itemCount; i++) {
        display.setCursor(2, 12 + i * 12);
        
        if (i == selectedIndex) {
            display.print(">");
        } else {
            display.print(" ");
        }
        
        display.println(menuItems[i]);
    }
    
    drawFooter("A:Execute B:Back", "Enc:Navigate");
    update();
}

void DisplayManager::showMessage(const String& title, const String& message, bool success) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader(title, false);
    
    display.setCursor(0, 15);
    display.println(message);
    
    display.setCursor(50, 40);
    display.println(success ? "✅ Success" : "❌ Failed");
    
    drawFooter("Press any button", "to continue");
    update();
}

void DisplayManager::showError(const String& title, const String& message) {
    showMessage("❌ " + title, message, false);
}

void DisplayManager::showScanningAnimation(const String& message) {
    if (!displayAvailable) return;
    
    clear();
    drawHeader("🔍 SCANNING", false);
    
    display.setCursor(0, 20);
    display.println(message);
    
    // Simple animation
    int dotCount = (millis() / 500) % 4;
    String dots = "";
    for (int i = 0; i < dotCount; i++) {
        dots += ".";
    }
    
    display.setCursor(50, 40);
    display.println(dots);
    
    update();
}

// Utility functions
void DisplayManager::setCursor(int x, int y) {
    if (!displayAvailable) return;
    display.setCursor(x, y);
}

void DisplayManager::print(const String& text) {
    if (!displayAvailable) return;
    display.print(text);
}

void DisplayManager::println(const String& text) {
    if (!displayAvailable) return;
    display.println(text);
}

void DisplayManager::drawHorizontalLine(int y) {
    if (!displayAvailable) return;
    display.drawLine(0, y, SCREEN_WIDTH, y, SH110X_WHITE);
}

void DisplayManager::drawVerticalLine(int x) {
    if (!displayAvailable) return;
    display.drawLine(x, 0, x, SCREEN_HEIGHT, SH110X_WHITE);
}

void DisplayManager::drawBatteryIcon(int x, int y, int level) {
    if (!displayAvailable) return;
    // Simple battery icon implementation
    display.drawRect(x, y, 15, 8, SH110X_WHITE);
    display.fillRect(x + 15, y + 2, 2, 4, SH110X_WHITE);
    
    int fillWidth = map(level, 0, 100, 0, 13);
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, 6, SH110X_WHITE);
    }
}

void DisplayManager::drawWiFiIcon(int x, int y, bool connected, int rssi) {
    if (!displayAvailable) return;
    
    if (connected) {
        if (rssi > -50) {
            display.print("📶"); // Strong signal
        } else if (rssi > -70) {
            display.print("📶"); // Medium signal  
        } else {
            display.print("📶"); // Weak signal
        }
    } else {
        display.print("❌");
    }
}

void DisplayManager::drawClockIcon(int x, int y, bool synced) {
    if (!displayAvailable) return;
    display.print(synced ? "🕐" : "⏰");
}