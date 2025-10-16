#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "display_manager.h"
#include "menu_system.h"
#include "time_manager.h"
#include "calibration_manager.h"

// ==================== GLOBAL VARIABLES ====================
unsigned long lastSensorRead = 0;
unsigned long lastDataPost = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastDisplayUpdate = 0;

bool systemInitialized = false;
String systemStatus = "Booting...";

// ==================== SETUP ====================
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  
  Utils::info("=== ESP32-S3 Water Quality Monitor ===");
  Utils::info("Device: " + String(DEVICE_NAME));
  Utils::info("UID: " + String(DEVICE_UID));
  Utils::info("Chip ID: " + Utils::getChipID());
  
  // Initialize display first for user feedback
  if (displayManager.begin()) {
    displayManager.showSplashScreen();
    delay(2000);
  }
  
  // Initialize components
  bool timeOK = timeManager.begin();
  bool sensorsOK = sensorManager.begin();
  bool wifiOK = wifiManager.begin();
  
  // Build status message
  systemStatus = "";
  if (timeOK) systemStatus += "Time✓ ";
  if (sensorsOK) systemStatus += "Sensors✓ ";
  if (wifiOK) systemStatus += "WiFi✓ ";
  
  // Initialize menu system
  menuSystem.begin();
  
  systemInitialized = true;
  
  if (displayManager.begin()) {
    displayManager.showMessage("System Ready", systemStatus, true);
    delay(2000);
  }
  
  Utils::info("Setup completed: " + systemStatus);
}

// ==================== MAIN LOOP ====================
void loop() {
  unsigned long currentMillis = millis();
  
  // Always update menu system (handles encoder and buttons)
  menuSystem.update();
  
  // Read sensors periodically (when not in calibration)
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL && 
      !calibrationManager.isCalibrating() &&
      menuSystem.getCurrentState() != MENU_CALIBRATION_PROGRESS) {
    if (sensorManager.readAllSensors()) {
      lastSensorRead = currentMillis;
    }
  }
  
  // Send data to server periodically
  if (currentMillis - lastDataPost >= DATA_POST_INTERVAL && 
      wifiManager.isConnected() && 
      !calibrationManager.isCalibrating()) {
    String jsonPayload = sensorManager.getJSONPayload();
    if (wifiManager.sendDataToServer(jsonPayload)) {
      lastDataPost = currentMillis;
      Utils::info("Data sent to server successfully");
    }
  }
  
  // Handle WiFi reconnection
  if (currentMillis - lastWiFiCheck >= 10000) {
    wifiManager.handleReconnection();
    lastWiFiCheck = currentMillis;
  }
  
  // Small delay for stability
  delay(10);
}