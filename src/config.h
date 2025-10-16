#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==================== DEBUG SETTINGS ====================
#define DEBUG_MODE true
#define SERIAL_BAUD 115200

// ==================== PIN DEFINITIONS ====================
// RS485 Configuration
#define RS485_RX_PIN 16
#define RS485_TX_PIN 15
#define RS485_DE_RE_PIN 14

// OneWire Temperature Sensor
#define ONE_WIRE_BUS 13

// OLED Display (I2C) - Module specific
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1

// Encoder Pins - Sesuai modul OLED+EC11
#define ENCODER_CLK 39    // EC11 Pin A
#define ENCODER_DT 40     // EC11 Pin B
#define ENCODER_SW 41     // EC11 Push Button
#define BUTTON_A 42       // Additional Button 1
#define BUTTON_B 43       // Additional Button 2

// ==================== SENSOR ADDRESSES ====================
#define PH_SENSOR_ID 1
#define DO_SENSOR_ID 55
#define EC_SENSOR_ID 1
#define NH4_SENSOR_ID 1

// ==================== DEFAULT SETTINGS ====================
#define DEFAULT_TIMEZONE 7  // GMT+7
#define DEFAULT_POST_INTERVAL 30000  // 30 seconds
#define DEVICE_UID "AER2023AQ0015"
#define DEVICE_NAME "WaterQualityMonitor"

// ==================== DISPLAY SETTINGS ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

// ==================== MENU SETTINGS ====================
#define MENU_ITEMS 8
#define MAX_VISIBLE_ITEMS 4
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64

// ==================== ENCODER SETTINGS ====================
#define ENCODER_STEPS 4  // EC11 has 20 pulses per rotation
#define DEBOUNCE_DELAY 300

// ==================== TIMING SETTINGS ====================
#define SENSOR_READ_INTERVAL 5000     // 5 seconds
#define DATA_POST_INTERVAL 30000      // 30 seconds
#define DISPLAY_UPDATE_INTERVAL 500   // 500 ms
#define WIFI_RECONNECT_INTERVAL 30000 // 30 seconds

// ==================== NTP SETTINGS ====================
#define NTP_SERVER "pool.ntp.org"
#define NTP_TIMEOUT 10000

// ==================== API SETTINGS ====================
#define API_URL "https://aeraseaku.inkubasistartupunhas.id/sensor/"
#define API_TIMEOUT 10000

// ==================== ERROR CODES ====================
enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_SENSOR_COMM = 1,
    ERROR_WIFI_CONN = 2,
    ERROR_TIME_SYNC = 3,
    ERROR_SERVER_COMM = 4,
    ERROR_MEMORY = 5
};

#endif