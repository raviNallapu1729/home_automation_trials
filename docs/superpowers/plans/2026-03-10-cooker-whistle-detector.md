# Cooker Whistle Detector Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build STM32 firmware that detects pressure cooker whistles via microphone, counts them, and sends Alexa notifications when rice is ready.

**Architecture:** Edge-first hybrid - STM32 handles all audio processing and detection locally. Cloud only for Alexa notification delivery via Notify Me API. Three-stage detection: Energy+ZCR filter → Goertzel frequency check → temporal pattern confirmation.

**Tech Stack:** STM32 HAL, C, STM32CubeIDE, MEMS microphone (I2S/PDM), WiFi module, lightweight HTTP server, HTTPS client

---

## Chunk 1: Project Setup and Basic Infrastructure

### File Structure Overview

This project will create:

**Firmware (STM32 C code):**
- `firmware/Core/Inc/config.h` - Configuration constants and defaults
- `firmware/Core/Inc/types.h` - Type definitions and structures
- `firmware/Core/Src/main.c` - Main loop and initialization
- `firmware/Core/Src/audio_acquisition.c` - Microphone DMA capture
- `firmware/Core/Inc/audio_acquisition.h` - Audio module interface
- `firmware/Core/Src/whistle_detector.c` - Three-stage detection pipeline
- `firmware/Core/Inc/whistle_detector.h` - Detector interface
- `firmware/Core/Src/state_manager.c` - Whistle counting and session logic
- `firmware/Core/Inc/state_manager.h` - State manager interface
- `firmware/Core/Src/led_controller.c` - LED patterns
- `firmware/Core/Inc/led_controller.h` - LED interface
- `firmware/Core/Src/logger.c` - Event logging system
- `firmware/Core/Inc/logger.h` - Logger interface

**Web Interface:**
- `web_interface/index.html` - Configuration page with embedded CSS/JS

**Tests (host-based unit tests):**
- `tests/test_whistle_detector.c` - Detection algorithm tests
- `tests/test_state_manager.c` - State logic tests
- `tests/Makefile` - Host-based test build

**Documentation:**
- `README.md` - Project overview and setup instructions

---

### Task 1: Project Initialization and README

**Files:**
- Create: `README.md`

- [ ] **Step 1: Initialize git repository**

```bash
cd /Users/ravi.nallapu/personal_projs/home_automation_trials
git init
```

- [ ] **Step 2: Create README with project overview**

```markdown
# Pressure Cooker Whistle Detector

Edge-first IoT device that detects pressure cooker whistles and sends Alexa notifications.

## Hardware
- STM32 IoT Discovery Board (B-L475E-IOT01A)
- Digital MEMS microphone (onboard)
- WiFi module (onboard)

## Features
- Three-stage audio detection (Energy+ZCR, Goertzel, Temporal)
- Configurable whistle count threshold (default: 5)
- Web interface for configuration
- Alexa Notify Me integration
- Works offline (notifications require WiFi)

## Development Setup

### Prerequisites
- STM32CubeIDE (free from ST)
- STM32 HAL libraries
- USB cable for programming/debug
- Serial terminal (115200 baud)

### Build
1. Open project in STM32CubeIDE
2. Build: Project > Build All
3. Flash: Run > Debug

### Testing
```bash
cd tests
make
./run_tests
```

## Configuration

### WiFi Setup
1. Power on device (LED blinks slowly)
2. Use SmartConfig app or press WPS button on router
3. Device connects (LED solid)

### Web Interface
- Access: `http://cooker-monitor.local` or device IP
- Configure whistle threshold, timeout, sensitivity
- Test alert delivery

### Alexa Setup
1. Enable "Notify Me" skill in Alexa app
2. Get access code from skill
3. Enter code in web interface

## Architecture

```
Microphone (8kHz) → Stage1: Energy+ZCR → Stage2: Goertzel → Stage3: Temporal
                                                                      ↓
                                                              State Manager
                                                                      ↓
                                                          Notification Client
                                                                      ↓
                                                            Alexa Notify Me
```

## Design Document
See `docs/superpowers/specs/2026-03-10-cooker-whistle-detector-design.md`

## License
MIT
```

- [ ] **Step 3: Commit README**

```bash
git add README.md
git commit -m "docs: add project README with overview and setup"
```

---

### Task 2: Configuration Header (config.h)

**Files:**
- Create: `firmware/Core/Inc/config.h`

- [ ] **Step 1: Create config header with all constants**

```c
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
```

- [ ] **Step 2: Commit config header**

```bash
git add firmware/Core/Inc/config.h
git commit -m "feat: add configuration constants header"
```

---

### Task 3: Type Definitions (types.h)

**Files:**
- Create: `firmware/Core/Inc/types.h`

- [ ] **Step 1: Create types header with structures and enums**

```c
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

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
```

- [ ] **Step 2: Commit types header**

```bash
git add firmware/Core/Inc/types.h
git commit -m "feat: add type definitions and structures"
```

---

## Chunk 2: Audio Acquisition Module

### Task 4: Audio Acquisition Header

**Files:**
- Create: `firmware/Core/Inc/audio_acquisition.h`

- [ ] **Step 1: Create audio acquisition interface**

```c
#ifndef AUDIO_ACQUISITION_H
#define AUDIO_ACQUISITION_H

#include "types.h"
#include "config.h"

// Initialize audio acquisition (I2S/PDM DMA)
// Returns: 0 on success, -1 on error
int audio_init(void);

// Start audio acquisition
// Returns: 0 on success, -1 on error
int audio_start(void);

// Stop audio acquisition
void audio_stop(void);

// Check if new audio buffer is ready
// Returns: true if buffer ready, false otherwise
bool audio_buffer_ready(void);

// Get pointer to current audio buffer
// Returns: pointer to buffer or NULL if not ready
audio_buffer_t* audio_get_buffer(void);

// Mark current buffer as processed (releases for DMA)
void audio_release_buffer(void);

// Get audio status
// Returns: true if audio system operational
bool audio_is_healthy(void);

// Audio error callback (called from ISR)
void audio_error_callback(void);

// DMA half complete callback (called from ISR)
void audio_dma_half_complete(void);

// DMA full complete callback (called from ISR)
void audio_dma_full_complete(void);

#endif // AUDIO_ACQUISITION_H
```

- [ ] **Step 2: Commit audio header**

```bash
git add firmware/Core/Inc/audio_acquisition.h
git commit -m "feat: add audio acquisition module interface"
```

---

### Task 5: Audio Acquisition Implementation (Stub)

**Files:**
- Create: `firmware/Core/Src/audio_acquisition.c`

**Note:** This is an intentional stub implementation with TODO markers for HAL-specific code. Full I2S/PDM DMA integration will be completed in later chunks after hardware abstraction is set up. This stub allows development of higher-level modules while hardware integration is pending.

- [ ] **Step 1: Create audio module with stub implementation**

```c
#include "audio_acquisition.h"
#include <string.h>
#include <stdlib.h>

// Double buffering
static audio_buffer_t buffers[BUFFER_COUNT];
static uint8_t current_buffer = 0;
static uint8_t ready_buffer = 0xFF;  // No buffer ready
static bool audio_running = false;
static uint32_t last_sample_time = 0;

// Initialize audio acquisition
int audio_init(void) {
    // Clear buffers
    memset(buffers, 0, sizeof(buffers));
    current_buffer = 0;
    ready_buffer = 0xFF;
    audio_running = false;

    // TODO: Initialize I2S/PDM interface
    // TODO: Configure DMA for circular mode
    // TODO: Set up microphone clock and data pins

    return 0;
}

// Start audio acquisition
int audio_start(void) {
    if (audio_running) {
        return 0;  // Already running
    }

    // TODO: Start I2S/PDM DMA transfer
    // TODO: Enable microphone

    audio_running = true;
    last_sample_time = HAL_GetTick();  // Will be actual HAL call

    return 0;
}

// Stop audio acquisition
void audio_stop(void) {
    if (!audio_running) {
        return;
    }

    // TODO: Stop I2S/PDM DMA
    // TODO: Disable microphone

    audio_running = false;
}

// Check if buffer ready
bool audio_buffer_ready(void) {
    return (ready_buffer != 0xFF);
}

// Get current buffer
audio_buffer_t* audio_get_buffer(void) {
    if (ready_buffer == 0xFF) {
        return NULL;
    }

    return &buffers[ready_buffer];
}

// Release buffer
void audio_release_buffer(void) {
    ready_buffer = 0xFF;
}

// Check audio health
bool audio_is_healthy(void) {
    if (!audio_running) {
        return false;
    }

    // Check if we've received samples recently
    uint32_t now = HAL_GetTick();  // Will be actual HAL call
    uint32_t elapsed = now - last_sample_time;

    return (elapsed < (AUDIO_TIMEOUT_SEC * 1000));
}

// DMA half complete callback
void audio_dma_half_complete(void) {
    // First half of buffer filled
    buffers[0].timestamp_ms = HAL_GetTick();  // Will be actual HAL call
    buffers[0].ready = true;
    ready_buffer = 0;
    last_sample_time = buffers[0].timestamp_ms;
}

// DMA full complete callback
void audio_dma_full_complete(void) {
    // Second half of buffer filled
    buffers[1].timestamp_ms = HAL_GetTick();  // Will be actual HAL call
    buffers[1].ready = true;
    ready_buffer = 1;
    last_sample_time = buffers[1].timestamp_ms;
}

// Error callback
void audio_error_callback(void) {
    // TODO: Handle DMA errors
    audio_running = false;
}
```

- [ ] **Step 2: Commit audio stub implementation**

```bash
git add firmware/Core/Src/audio_acquisition.c
git commit -m "feat: add audio acquisition stub implementation"
```

---

### Task 6: Test Infrastructure Setup

**Files:**
- Create: `tests/Makefile`
- Create: `tests/README.md`

- [ ] **Step 1: Create tests directory**

```bash
mkdir -p tests
```

- [ ] **Step 2: Create basic test Makefile stub**

```makefile
# Test Makefile - Host-based unit tests
CC = gcc
CFLAGS = -Wall -Wextra -g -I../firmware/Core/Inc

# Test targets will be added in later chunks
.PHONY: all clean

all:
	@echo "Test framework ready. Specific tests will be added in later chunks."

clean:
	rm -f *.o test_*
```

- [ ] **Step 3: Create test README**

```markdown
# Unit Tests

Host-based unit tests for firmware modules.

## Running Tests
```bash
make
./run_tests
```

## Test Coverage
- Whistle detector algorithms (added in Chunk 2)
- State management logic (added in Chunk 3)
- Integration tests (added in Chunk 5)

Actual test implementation will be added alongside module development.
```

- [ ] **Step 4: Commit test infrastructure**

```bash
git add tests/
git commit -m "test: add test infrastructure stub"
```

---

End of Chunk 1. Ready for review before continuing with Chunk 2 (Whistle Detector Module).
