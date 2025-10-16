#include "menu_system.h"

MenuSystem menuSystem;

MenuSystem::MenuSystem() : 
    encoder(ENCODER_CLK, ENCODER_DT),
    currentState(MENU_MAIN),
    previousState(MENU_MAIN),
    encoderPosition(0),
    lastEncoderPosition(0),
    selectedItem(0),
    scrollOffset(0),
    lastButtonPress(0) {
}

void MenuSystem::begin() {
    pinMode(ENCODER_SW, INPUT_PULLUP);
    pinMode(BUTTON_A, INPUT_PULLUP);
    pinMode(BUTTON_B, INPUT_PULLUP);
    
    Serial.println("Menu System Initialized");
    Serial.println("Controls: Encoder + 3 buttons");
}

void MenuSystem::update() {
    handleEncoder();
    handleButtons();
    
    // Update display based on current state
    switch (currentState) {
        case MENU_MAIN:
            handleMainMenu();
            break;
        case MENU_SENSOR_DISPLAY:
            handleSensorDisplay();
            break;
        case MENU_SENSOR_DETAIL:
            handleSensorDetail();
            break;
        case MENU_WIFI_CONFIG:
            handleWiFiConfig();
            break;
        case MENU_CALIBRATION:
            handleCalibration();
            break;
        case MENU_CALIBRATION_PROGRESS:
            handleCalibrationProgress();
            break;
        case MENU_TIME_CONFIG:
            handleTimeConfig();
            break;
        case MENU_SYSTEM_INFO:
            handleSystemInfo();
            break;
        case MENU_SETTINGS:
            handleSettings();
            break;
        case MENU_MESSAGE:
            handleMessage();
            break;
    }
}

void MenuSystem::handleEncoder() {
    long newPosition = encoder.read() / ENCODER_STEPS;
    
    if (newPosition != lastEncoderPosition) {
        int direction = (newPosition > lastEncoderPosition) ? 1 : -1;
        
        switch (currentState) {
            case MENU_MAIN:
            case MENU_SENSOR_DETAIL:
            case MENU_CALIBRATION:
            case MENU_SETTINGS:
                selectedItem = constrain(selectedItem + direction, 0, 
                    currentState == MENU_MAIN ? 7 : 
                    currentState == MENU_SENSOR_DETAIL ? 5 :
                    currentState == MENU_CALIBRATION ? 6 : 3);
                
                // Adjust scroll offset
                if (selectedItem < scrollOffset) {
                    scrollOffset = selectedItem;
                } else if (selectedItem >= scrollOffset + MAX_VISIBLE_ITEMS) {
                    scrollOffset = selectedItem - MAX_VISIBLE_ITEMS + 1;
                }
                break;
                
            case MENU_TIME_CONFIG:
                timeManager.setTimezone(constrain(timeManager.getTimezone() + direction, 0, 14));
                break;
        }
        
        lastEncoderPosition = newPosition;
    }
}

void MenuSystem::handleButtons() {
    // Read current button states
    bool currentEncoderBtn = !digitalRead(ENCODER_SW);
    bool currentButtonA = !digitalRead(BUTTON_A);
    bool currentButtonB = !digitalRead(BUTTON_B);
    
    unsigned long currentTime = millis();
    
    // Check for button presses (with debounce)
    if (currentTime - lastButtonPress > DEBOUNCE_DELAY) {
        if (currentEncoderBtn && !lastEncoderButtonState) {
            handleButtonAction(0); // Encoder button
        } else if (currentButtonA && !lastButtonAState) {
            handleButtonAction(1); // Button A
        } else if (currentButtonB && !lastButtonBState) {
            handleButtonAction(2); // Button B
        }
        
        if (currentEncoderBtn || currentButtonA || currentButtonB) {
            lastButtonPress = currentTime;
        }
    }
    
    // Update last states
    lastEncoderButtonState = currentEncoderBtn;
    lastButtonAState = currentButtonA;
    lastButtonBState = currentButtonB;
}

void MenuSystem::handleButtonAction(int button) {
    switch (button) {
        case 0: // Encoder button - Select/OK
            executeMenuItem();
            break;
            
        case 1: // Button A - Action/Context
            switch (currentState) {
                case MENU_SENSOR_DISPLAY:
                    sensorManager.readAllSensors(); // Manual refresh
                    break;
                case MENU_WIFI_CONFIG:
                    wifiManager.scanNetworks();
                    break;
                case MENU_TIME_CONFIG:
                    timeManager.forceResync();
                    break;
                case MENU_CALIBRATION_PROGRESS:
                    calibrationManager.cancelCalibration();
                    navigateTo(MENU_CALIBRATION);
                    break;
            }
            break;
            
        case 2: // Button B - Back/Cancel
            goBack();
            break;
    }
}

void MenuSystem::executeMenuItem() {
    switch (currentState) {
        case MENU_MAIN:
            switch (selectedItem) {
                case 0: navigateTo(MENU_SENSOR_DISPLAY); break;
                case 1: navigateTo(MENU_SENSOR_DETAIL); break;
                case 2: navigateTo(MENU_WIFI_CONFIG); break;
                case 3: navigateTo(MENU_CALIBRATION); break;
                case 4: navigateTo(MENU_TIME_CONFIG); break;
                case 5: navigateTo(MENU_SYSTEM_INFO); break;
                case 6: navigateTo(MENU_SETTINGS); break;
                case 7: 
                    sensorManager.readAllSensors();
                    showMessage("Data Refresh", "Sensor data updated", true);
                    break;
            }
            break;
            
        case MENU_SENSOR_DETAIL:
            // Stay in detail view, just show selected sensor
            break;
            
        case MENU_CALIBRATION:
            if (selectedItem >= 0 && selectedItem <= 6) {
                CalibrationType calibType = static_cast<CalibrationType>(selectedItem + 1);
                calibrationManager.beginCalibration(calibType);
                navigateTo(MENU_CALIBRATION_PROGRESS);
            }
            break;
            
        case MENU_SETTINGS:
            switch (selectedItem) {
                case 0: // Factory Reset
                    showMessage("Factory Reset", "All settings will be erased", false);
                    // Implementation would go here
                    break;
                case 1: // Reboot
                    ESP.restart();
                    break;
                case 2: // Data Interval
                    showMessage("Data Interval", "Adjust data logging interval", true);
                    break;
                case 3: // Device Lock
                    showMessage("Device Lock", "Device locked", true);
                    break;
            }
            break;
    }
}

void MenuSystem::goBack() {
    if (currentState == MENU_MAIN) {
        // Can't go back from main menu
        return;
    }
    
    if (calibrationManager.isCalibrating()) {
        calibrationManager.cancelCalibration();
    }
    
    navigateTo(MENU_MAIN);
}

void MenuSystem::navigateTo(MenuState newState) {
    previousState = currentState;
    currentState = newState;
    selectedItem = 0;
    scrollOffset = 0;
}

// State handler implementations
void MenuSystem::handleMainMenu() {
    displayManager.showMainMenu(mainMenu, 8, selectedItem, scrollOffset);
}

void MenuSystem::handleSensorDisplay() {
    SensorData data = sensorManager.getSensorData();
    displayManager.showSensorData(data);
}

void MenuSystem::handleSensorDetail() {
    SensorData data = sensorManager.getSensorData();
    displayManager.showSensorDetail(data, selectedItem);
}

void MenuSystem::handleWiFiConfig() {
    displayManager.showWiFiConfig(
        wifiManager.getSSID(),
        wifiManager.isConnected() ? "Connected" : "Disconnected",
        wifiManager.getIPAddress(),
        wifiManager.getRSSI(),
        wifiManager.isAPMode()
    );
}

void MenuSystem::handleCalibration() {
    displayManager.showCalibrationMenu(calibrationMenu, 7, selectedItem, scrollOffset);
}

void MenuSystem::handleCalibrationProgress() {
    if (!calibrationManager.isCalibrating()) {
        navigateTo(MENU_CALIBRATION);
        return;
    }
    
    displayManager.showCalibrationProgress(
        calibrationManager.getSensorName(calibrationManager.getCurrentCalibration()),
        calibrationManager.getInstruction(),
        calibrationManager.getCurrentValue(),
        calibrationManager.getProgress()
    );
    
    calibrationManager.updateCalibration();
}

void MenuSystem::handleTimeConfig() {
    displayManager.showTimeConfig(
        timeManager.getTimezone(),
        timeManager.getFormattedTime(),
        timeManager.isTimeSynced()
    );
}

void MenuSystem::handleSystemInfo() {
    SensorData data = sensorManager.getSensorData();
    int workingSensors = 6;
    if (data.ds18b20_error) workingSensors--;
    if (data.ph_error) workingSensors--;
    if (data.do_error) workingSensors--;
    if (data.ec_error) workingSensors--;
    if (data.nh4_error) workingSensors--;
    
    displayManager.showSystemInfo(data, timeManager.getUptime(), workingSensors);
}

void MenuSystem::handleSettings() {
    displayManager.showSettingsMenu(settingsMenu, 4, selectedItem);
}

void MenuSystem::handleMessage() {
    // Message screen is handled by showMessage method
}

void MenuSystem::showMessage(const String& title, const String& message, bool success) {
    previousState = currentState;
    currentState = MENU_MESSAGE;
    displayManager.showMessage(title, message, success);
}

MenuState MenuSystem::getCurrentState() {
    return currentState;
}