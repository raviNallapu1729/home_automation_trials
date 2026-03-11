#ifndef CONFIG_H
#define CONFIG_H

// Audio Configuration
#define SAMPLE_RATE_HZ              8000
#define BUFFER_SIZE_SAMPLES         512
#define BUFFER_SIZE_BYTES           (BUFFER_SIZE_SAMPLES * 2)  // 16-bit samples
#define BUFFER_COUNT                2  // Double buffering

// Stage 1: Energy + ZCR Filter
// Note: Energy thresholds map to sensitivity_t enum values:
//   SENSITIVITY_LOW -> ENERGY_THRESHOLD_LOW (20%)
//   SENSITIVITY_MEDIUM -> ENERGY_THRESHOLD_MEDIUM (40%, default)
//   SENSITIVITY_HIGH -> ENERGY_THRESHOLD_HIGH (60%)
#define ENERGY_THRESHOLD_LOW        0.20f  // 20% of max amplitude
#define ENERGY_THRESHOLD_MEDIUM     0.40f  // 40% of max amplitude (default)
#define ENERGY_THRESHOLD_HIGH       0.60f  // 60% of max amplitude
#define ZCR_MIN_RATE                2000   // Crossings per second
#define ZCR_MAX_RATE                4500   // Crossings per second

// Stage 2: Goertzel Frequency Detection
#define GOERTZEL_FREQ_1             2500   // Hz - lower whistle
#define GOERTZEL_FREQ_2             3000   // Hz - typical whistle
#define GOERTZEL_FREQ_3             3500   // Hz - upper whistle
#define GOERTZEL_BACKGROUND_FREQ_1  1000   // Hz - low background
#define GOERTZEL_BACKGROUND_FREQ_2  6000   // Hz - high background
#define GOERTZEL_SNR_THRESHOLD      3.0f   // 3:1 signal to noise ratio

// Stage 3: Temporal Pattern
#define WHISTLE_MIN_DURATION_MS     500    // 0.5 seconds minimum
#define WHISTLE_MAX_DURATION_MS     10000  // 10 seconds maximum
#define TEMPORAL_CONFIRMATION_PCT   70     // 70% of windows must pass

// State Management
#define DEFAULT_N_CUTOFF            5      // Default whistle count
#define DEFAULT_TIMEOUT_WINDOW_SEC  300    // 5 minutes between whistles
#define MIN_N_CUTOFF                1
#define MAX_N_CUTOFF                10
#define MIN_TIMEOUT_WINDOW_SEC      180    // 3 minutes
#define MAX_TIMEOUT_WINDOW_SEC      600    // 10 minutes

// LED Patterns (milliseconds)
#define LED_FLASH_QUICK_MS          100
#define LED_FLASH_DOUBLE_MS         100    // Each flash in double pattern
#define LED_FLASH_DOUBLE_GAP_MS     100    // Gap between flashes
#define LED_FLASH_TRIPLE_MS         100
#define LED_BLINK_SLOW_MS           1000
#define LED_BLINK_FAST_MS           200
#define LED_BLINK_RAPID_MS          50

// WiFi Configuration
#define WIFI_RETRY_MAX              5
#define WIFI_RETRY_DELAY_MS         10000  // 10 seconds
#define WIFI_MONITOR_INTERVAL_MS    30000  // 30 seconds

// HTTP Server
#define HTTP_SERVER_PORT            80
#define HTTP_MAX_CONNECTIONS        2

// Alexa Notify Me
#define ALEXA_API_URL               "https://api.notifymyecho.com/v1/NotifyMe"
#define ALEXA_RETRY_MAX             3
#define ALEXA_RETRY_DELAY_MS        30000  // 30 seconds
#define ALEXA_DEFAULT_MESSAGE       "Your rice is ready!"

// Logging
#define LOG_BUFFER_SIZE             100    // Circular buffer entries
#define LOG_FLASH_SIZE              50     // Critical logs in flash

// Error Detection
#define AUDIO_TIMEOUT_SEC           5      // No samples = error
#define FALSE_DETECTION_THRESHOLD   10     // Whistles per minute
#define FALSE_DETECTION_WINDOW_MS   60000  // 1 minute
#define WATCHDOG_TIMEOUT_MS         10000  // 10 seconds
#define REBOOT_THRESHOLD            5      // Reboots per hour for safe mode

// Memory
#define AUDIO_BUFFER_RAM_KB         2      // ~2KB for double buffer
#define GOERTZEL_RAM_KB             2      // ~2KB for frequency bins
#define STATE_RAM_BYTES             256    // State variables
#define LOG_RAM_KB                  4      // ~4KB for log buffer

#endif // CONFIG_H
