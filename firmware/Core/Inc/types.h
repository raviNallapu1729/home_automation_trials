#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Sensitivity levels
typedef enum {
    SENSITIVITY_LOW = 0,
    SENSITIVITY_MEDIUM = 1,
    SENSITIVITY_HIGH = 2
} sensitivity_t;

// Device operating modes
typedef enum {
    MODE_SETUP = 0,      // WiFi configuration
    MODE_IDLE,           // Connected, waiting
    MODE_LISTENING,      // Stage 1 active
    MODE_DETECTING,      // Stages 2-3 active
    MODE_COUNTING,       // Tracking whistles
    MODE_ALERTING,       // Sending notification
    MODE_ERROR,          // Error state
    MODE_SAFE            // Safe mode
} device_mode_t;

// LED states
typedef enum {
    LED_SOLID_ON = 0,
    LED_QUICK_FLASH,
    LED_DOUBLE_FLASH,
    LED_TRIPLE_FLASH,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_RAPID_FLASH
} led_state_t;

// Detection stage results
typedef enum {
    STAGE_PASS = 0,
    STAGE_FAIL = 1
} stage_result_t;

// Log event types
typedef enum {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} log_level_t;

typedef enum {
    EVENT_WHISTLE_DETECTED = 0,
    EVENT_ALERT_SENT,
    EVENT_ALERT_FAILED,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_CONFIG_CHANGED,
    EVENT_SESSION_STARTED,
    EVENT_SESSION_ENDED,
    EVENT_ERROR_MICROPHONE,
    EVENT_ERROR_MEMORY,
    EVENT_SYSTEM_BOOT
} event_type_t;

// Audio buffer structure
typedef struct {
    int16_t samples[BUFFER_SIZE_SAMPLES];
    uint32_t timestamp_ms;
    bool ready;
} audio_buffer_t;

// Detection stage 1 result
typedef struct {
    float energy;
    uint32_t zero_crossings;
    float zcr_rate;
    stage_result_t result;
} stage1_result_t;

// Detection stage 2 result
typedef struct {
    float magnitude_2500hz;
    float magnitude_3000hz;
    float magnitude_3500hz;
    float background_1000hz;
    float background_6000hz;
    float snr;
    stage_result_t result;
} stage2_result_t;

// Detection stage 3 result
typedef struct {
    uint32_t duration_ms;
    uint32_t passed_windows;
    uint32_t total_windows;
    float confirmation_pct;
    bool whistle_detected;
} stage3_result_t;

// Device state structure
typedef struct {
    uint8_t whistle_count;
    uint8_t n_cutoff;
    uint32_t timeout_window_sec;
    sensitivity_t sensitivity;
    uint32_t last_whistle_time_ms;
    bool session_active;
    device_mode_t mode;
    led_state_t led_state;
} device_state_t;

// Persistent configuration (stored in flash)
typedef struct {
    uint8_t n_cutoff;
    uint32_t timeout_window_sec;
    sensitivity_t sensitivity;
    char alexa_access_code[64];
    uint32_t magic;  // Validation marker
    uint32_t crc;    // Integrity check
} persistent_config_t;

// Log entry structure
typedef struct {
    uint32_t timestamp_ms;
    event_type_t event;
    log_level_t level;
    uint16_t data;  // Event-specific data
} log_entry_t;

// WiFi status
typedef struct {
    bool connected;
    int8_t rssi;
    char ip_address[16];
    uint32_t uptime_sec;
} wifi_status_t;

// System status (for web API)
typedef struct {
    device_mode_t mode;
    uint8_t whistle_count;
    uint8_t n_cutoff;
    uint32_t last_whistle_sec_ago;
    bool session_active;
    wifi_status_t wifi;
} system_status_t;

#endif // TYPES_H
