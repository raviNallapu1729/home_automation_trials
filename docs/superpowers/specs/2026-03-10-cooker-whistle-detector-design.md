# Pressure Cooker Whistle Detector Design Specification

**Project:** Home Automation Trials - Cooker Whistle Monitor
**Date:** 2026-03-10
**Platform:** STM32 IoT Discovery Board
**Architecture:** Edge-First Hybrid

---

## 1. Overview

### Purpose
Detect pressure cooker whistles using STM32 board's microphone, count whistles, and send Alexa notification when rice is ready (after n_cutoff whistles detected).

### Key Requirements
- Detect steam whistle sounds from Indian pressure cookers
- Count whistles with configurable threshold (default: 5)
- Handle extended whistle durations (0.5-10 seconds)
- Support long intervals between whistles (up to 3-5 minutes)
- Send Alexa notification when threshold reached
- Web interface for configuration
- Voice command support via Alexa for settings
- Work reliably with kitchen ambient noise

### Approach
Edge-First Hybrid: STM32 handles all detection and counting locally. Cloud services only used for Alexa notification delivery. Core functionality works offline.

---

## 2. System Architecture

### Hardware Components
- STM32 IoT Discovery Board (B-L475E-IOT01A or similar)
- Digital MEMS microphone (onboard)
- WiFi module (onboard)
- Single LED indicator (onboard)
- USB power supply

### Firmware Modules

**Audio Acquisition Module**
- Captures microphone data at 8 kHz sampling rate
- DMA-based continuous acquisition
- Double buffering (512 samples per buffer)

**Whistle Detector Module**
- Three-stage detection pipeline:
  1. Energy + Zero-Crossing Rate filter
  2. Goertzel frequency detection
  3. Temporal pattern confirmation

**State Manager Module**
- Tracks whistle count (0 to n_cutoff)
- Manages timeout windows (5 minutes between whistles)
- Handles session lifecycle
- Stores configuration

**Web Server Module**
- Lightweight HTTP server on port 80
- Serves configuration page
- RESTful API for status and settings

**Notification Client Module**
- Sends alerts to Alexa Notify Me API
- Handles retries and failures
- HTTPS client

**WiFi Manager Module**
- SmartConfig/WPS setup
- Connection monitoring
- Auto-reconnection

### External Services
- Alexa Notify Me Skill (free notification service)
- Home WiFi network

### Data Flow
```
Microphone (8 kHz)
  → Stage 1: Energy + ZCR Filter (continuous)
  → Stage 2: Goertzel Frequency Check (triggered)
  → Stage 3: Temporal Pattern (0.5-10s)
  → State Manager: Count + Timeout
  → Notification Client (at threshold)
  → Alexa Notify Me API
  → User's Alexa Device
```

### Operating Modes
1. **Setup Mode** - First boot, WiFi configuration
2. **Idle Mode** - Connected, waiting for cooking
3. **Active Listening** - Stage 1 running continuously
4. **Detection Mode** - Stages 2 & 3 active
5. **Counting Mode** - Tracking whistles with timeout
6. **Alert Mode** - Sending notification

---

## 3. Audio Processing Pipeline

### Sampling Configuration
- **Sample Rate:** 8 kHz (sufficient for 4 kHz Nyquist)
- **Bit Depth:** 16-bit signed integers
- **Buffer Size:** 512 samples (64ms windows)
- **Acquisition:** DMA-based, circular buffer
- **Memory:** ~2 KB for double buffering

### Stage 1: Energy + Zero-Crossing Rate Filter

**Purpose:** Fast pre-filter running continuously

**Algorithm:**
```
For each 64ms window (512 samples):
  1. Calculate RMS Energy = sqrt(sum(sample²) / N)
  2. Count Zero Crossings
  3. Calculate ZCR rate = crossings/second

  If (Energy > threshold) AND (ZCR in 2000-4500/sec):
      → Trigger Stage 2
  Else:
      → Continue monitoring
```

**Parameters:**
- Energy Threshold: Adaptive (~40% max, adjusts for ambient)
- ZCR Range: 2000-4500 crossings/sec
- CPU Usage: ~2-3%

**Rationale:** Filters out low-frequency sounds (talking, doors), low-energy sounds (background hum), and transient spikes (dishes clanking).

### Stage 2: Goertzel Frequency Detection

**Purpose:** Verify sound is in whistle frequency band (2-4 kHz)

**Algorithm:**
```
Apply Goertzel at three frequencies:
  - 2.5 kHz (lower whistle)
  - 3.0 kHz (typical peak)
  - 3.5 kHz (upper whistle)

Calculate background at 1 kHz and 6 kHz

If (whistle_band_magnitude > 3x background):
    → Trigger Stage 3
Else:
    → Return to Stage 1
```

**Parameters:**
- Block Size: 512 samples (reuse Stage 1 buffer)
- Frequency Bins: 2500, 3000, 3500 Hz
- Background Bins: 1000, 6000 Hz
- SNR Threshold: 3:1 ratio
- CPU Usage: ~5-8% when active

**Rationale:** Goertzel is more efficient than FFT for single-frequency detection. Leverages STM32 Cortex-M4 FPU. Background comparison eliminates broadband noise.

### Stage 3: Temporal Pattern Confirmation

**Purpose:** Confirm sustained whistle duration

**Algorithm:**
```
Start timer when Stage 2 first triggers

Continue Stage 2 on each 64ms window

Track duration where Stage 2 passes

If duration >= 500ms AND duration <= 10000ms:
    If whistle ends (Stage 2 fails):
        → WHISTLE DETECTED

If fails before 500ms:
    → False alarm, reset

If duration > 10s:
    → Continuous noise, ignore
```

**Parameters:**
- Minimum Duration: 500 ms
- Maximum Duration: 10,000 ms (Indian cookers)
- Confirmation Threshold: 70% windows pass Stage 2
- CPU Usage: ~1%

**Rationale:** Indian pressure cookers can whistle 5-10 seconds. Longer timeout accommodates higher pressure settings.

### Detection Tuning

**User-Adjustable (via web interface):**
- `n_cutoff`: Whistle count threshold (1-10, default: 5)
- `timeout_window`: Max time between whistles (180-600 sec, default: 300)
- `sensitivity`: Low/Medium/High (adjusts Stage 1 energy)

**Advanced (hidden):**
- Energy threshold multiplier
- ZCR range bounds
- Goertzel SNR threshold
- Min/max whistle duration

---

## 4. State Management

### State Variables

**Persistent (flash memory):**
- `n_cutoff`: Target count (default: 5)
- `timeout_window`: Seconds (default: 300)
- `sensitivity_level`: Low/Medium/High
- WiFi credentials (encrypted)
- Alexa access code

**Runtime (RAM):**
- `whistle_count`: Current count (0 to n_cutoff)
- `last_whistle_time`: Timestamp in milliseconds
- `session_active`: Boolean
- `led_state`: Current pattern

### Counting Logic

```
On WHISTLE_DETECTED:
  current_time = get_timestamp()

  If session_active:
      time_since_last = current_time - last_whistle_time

      If time_since_last <= timeout_window:
          whistle_count++
          last_whistle_time = current_time
          LED: Quick flash

          If whistle_count >= n_cutoff:
              → Send Alert
              → Reset session
      Else:
          // Timeout, new session
          whistle_count = 1
          last_whistle_time = current_time
          session_active = true
          LED: Double flash
  Else:
      // First whistle
      whistle_count = 1
      last_whistle_time = current_time
      session_active = true
      LED: Double flash
```

### Timeout Monitoring

```
Background task (every 10 seconds):
  If session_active:
      If (current_time - last_whistle_time) > timeout_window:
          session_active = false
          whistle_count = 0
          LED: Long slow flash
```

### LED Patterns

- **Solid ON:** Connected, idle
- **Quick flash:** Whistle detected
- **Double flash:** New session started
- **Triple flash:** Alert sent
- **Slow blink:** No WiFi
- **Fast blink:** Error
- **Rapid flash:** Factory reset mode

---

## 5. Web Interface

### HTTP Server
- Lightweight server on STM32
- Port 80 (HTTP, local network only)
- Static HTML from flash (~10 KB)
- RESTful API

### Configuration Page

**Access:** `http://cooker-monitor.local` or `http://192.168.x.x`

**Layout:**
```
Status Section:
  - Current state (Idle/Active/Alerting)
  - Whistle count: X / n_cutoff
  - Last whistle: N seconds ago

Settings Section:
  - Whistle count threshold: [3][4][5][6][7][8]
  - Timeout window: [3min][5min][10min]
  - Sensitivity: [Low][Medium][High]
  - [Save Settings] [Test Alert]

Device Info:
  - WiFi status and signal strength
  - IP address
  - Uptime
```

### API Endpoints

**GET `/api/status`**
```json
{
  "status": "active",
  "whistle_count": 2,
  "n_cutoff": 5,
  "last_whistle_seconds_ago": 45,
  "session_active": true,
  "wifi_connected": true,
  "wifi_rssi": -45
}
```

**POST `/api/config`**
```json
{
  "n_cutoff": 5,
  "timeout_window": 300,
  "sensitivity": "medium"
}
```

**POST `/api/test-alert`** - Send test notification

**GET `/api/reset`** - Reset whistle count

**GET `/api/logs`** - Get last 50 events

### Implementation
- Vanilla JavaScript (no frameworks)
- Auto-refresh status every 5 seconds
- Mobile responsive
- Settings saved to flash

---

## 6. Alexa Notification

### Service Integration

**Alexa Notify Me Skill:**
- Free third-party skill
- No AWS account needed
- One-time setup

**API Call:**
```
HTTP POST https://api.notifymyecho.com/v1/NotifyMe
Headers:
  Content-Type: application/json
Body:
  {
    "notification": "Your rice is ready!",
    "accessCode": "<user_access_code>"
  }
```

### Notification Flow

```
When whistle_count >= n_cutoff:
  POST to Notify Me API

  If success (200 OK):
    LED: Triple flash
    Log: Alert sent
    Reset counter
  Else:
    LED: Fast blink
    Retry after 30s (max 3 retries)
    If all fail: Reset anyway (prevent stuck state)
```

### Message Content

**Default:** "Your rice is ready!"

**Future:**
- Include count: "Ready after 5 whistles"
- Include timing: "Ready in 15 minutes"
- Customizable via web interface

### Fallback Behavior

If internet unavailable:
- LED shows error (fast blink)
- Retries 3 times over 2 minutes
- Logs event locally
- Resets counter to prevent stuck state
- User checks web interface for status

### Network Requirements
- Outbound HTTPS (port 443)
- No port forwarding
- ~1 KB per notification

---

## 7. WiFi Management

### Initial Setup (SmartConfig/WPS)

**First Boot:**
```
1. Power on → LED: Slow blink
2. Check flash for WiFi credentials
3. If none found:
   → Enter SmartConfig/WPS mode
   → LED: Fast double blink
   → Wait for WPS button or SmartConfig packet
4. Credentials received:
   → Save to flash (encrypted)
   → Connect to WiFi
5. Success:
   → LED: Solid ON
   → Start web server
```

**SmartConfig:**
- Uses ESP-TOUCH protocol
- Free phone apps available
- Broadcasts credentials locally
- Setup time: 30-60 seconds

**WPS:**
- Press WPS button on router
- Auto-detects on device
- Setup time: 10-20 seconds

### Runtime Management

**Connection Monitoring:**
```
Background task (every 30 seconds):
  If WiFi disconnected:
    LED: Slow blink
    Retry every 10s (max 5 attempts)

    Whistle detection continues (works offline)
    Notifications queued

  If reconnected:
    LED: Solid ON
    Send queued notifications
```

**Network Config:**
- DHCP client (auto IP)
- mDNS responder (`cooker-monitor.local`)
- Fallback static IP: 192.168.1.200

### Factory Reset

**Sequence:**
- Hold button 10 seconds
- LED: Rapid flash
- Clear WiFi credentials
- Reboot to setup mode

### Security
- Credentials encrypted in flash
- HTTP only on local network
- No external port exposure
- No remote access

---

## 8. Error Handling

### Critical Errors

**Microphone Failure:**
- Detection: No samples for 5 seconds
- Response: LED fast blink, reinit 3x, halt if fails
- Web: "Microphone Error"

**WiFi Lost During Session:**
- Continue whistle detection offline
- Queue notification
- LED: Slow blink (working without WiFi)
- Send when reconnected

**Alexa API Failure:**
- Retry 3x over 90 seconds
- LED: Fast blink during retries
- Log failure
- Reset counter anyway (prevent stuck)

**Memory Overflow:**
- Emergency stop audio
- LED: Rapid flash
- Log with stack trace
- Soft reset after 5 seconds
- Safe defaults on reboot

**False Detection Spam:**
- Detection: >10 whistles in 1 minute
- Response: Increase thresholds temporarily
- LED: Slow double blink
- Return to normal after 5 min quiet

### Watchdog Timer

- Hardware watchdog: 10 second timeout
- Must be kicked by main loop
- Auto reset on hang
- Increment reboot counter
- Safe mode if >5 reboots/hour

### Safe Mode

**Triggered by:** Repeated crashes

**Behavior:**
- Disable audio processing
- Run minimal web server
- LED: Alternating slow blink
- Show diagnostic info
- User can view logs, reset settings
- Manual exit button

### Logging

**Storage:**
- Last 100 events in RAM (circular buffer)
- Critical errors in flash (persistent)

**Events:**
- Whistle detections with timestamp
- Alert success/failure
- WiFi connect/disconnect
- Errors and warnings
- Config changes

**Access:** Web interface `/api/logs`

---

## 9. Testing Strategy

### Component Testing
- Audio acquisition: Verify 8 kHz sampling
- Detection stages: Feed recorded audio, verify output
- WiFi: Test SmartConfig/WPS
- Web server: Test all API endpoints
- Alexa: Test notification delivery

### Integration Testing
- Full pipeline: Real whistle → alert
- Negative tests: Kitchen sounds (dishes, talking, water)
- Timeout: Verify counter resets after 5 min
- Multi-whistle: Full cycle counting

### Real-World Testing
- Test with actual pressure cooker
- Various distances
- Kitchen ambient noise
- Alexa delivery reliability
- Tune thresholds

### Test Data
- Record 5-10 pressure cooker whistle samples
- Record common false positives
- Use for regression testing

### Success Criteria
- Detection rate: >90% actual whistles
- False positive rate: <15% in kitchen
- Alert delivery: >95% when WiFi available
- Response time: <10 seconds to alert

---

## 10. Implementation Plan

### Development Environment
- STM32CubeIDE (free)
- STM32 HAL libraries
- Git version control
- Serial monitor for debug

### Project Structure
```
home_automation_trials/
├── docs/
│   ├── stm32_board_specs.txt
│   ├── iot_device.jpg
│   └── superpowers/specs/
├── firmware/
│   ├── src/
│   │   ├── main.c
│   │   ├── audio_acquisition.c
│   │   ├── whistle_detector.c
│   │   ├── state_manager.c
│   │   ├── web_server.c
│   │   ├── notification_client.c
│   │   └── wifi_manager.c
│   └── inc/
├── web_interface/
│   └── index.html
└── README.md
```

### Deployment
1. Flash firmware via USB
2. Power on, configure WiFi (SmartConfig/WPS)
3. Access web page, configure settings
4. Enable Alexa Notify Me, enter access code
5. Test with manual whistle
6. Deploy near stove

### Future Enhancements
- Voice activation (low power wake-on-sound)
- Machine learning for improved accuracy
- SMS backup notification
- Multi-device support
- Cooking analytics
- Custom Alexa skill with voice config

---

## 11. Acoustic Characteristics Reference

### Pressure Cooker Whistles (Indian Cooking Context)

**Frequency:**
- Primary: 2-4 kHz (steam whistle physics)
- Peak energy: 2.5-3.5 kHz
- Harmonics up to 8-10 kHz

**Duration:**
- Range: 0.5-10 seconds per burst (Indian cookers sustain longer)
- Typical: 2-5 seconds
- Extended pressure: up to 10 seconds

**Pattern:**
- Repetitive bursts
- Intervals: 1-3 minutes between whistles (longer than Western cookers)
- Heat cycles as pressure rebuilds

**Amplitude:**
- 70-90 dB SPL at 1 meter
- High-pitched, distinctive
- Fluctuates with steam pressure

**Behavior:**
- Sharp attack (sudden onset)
- Sustained level during whistle
- May warble slightly

---

## Design Rationale Summary

**Why Edge-First Hybrid?**
- Core detection works offline (reliable)
- Privacy preserved (no streaming to cloud)
- Low cloud costs (minimal API calls)
- Easy to extend (can add features later)

**Why Three-Stage Detection?**
- Stage 1 (Energy+ZCR): Fast, low CPU, eliminates most noise
- Stage 2 (Goertzel): Accurate frequency check, efficient on STM32 FPU
- Stage 3 (Temporal): Confirms sustained duration, handles Indian cooker patterns
- Total: 85-95% accuracy, <15% false positives, 3-15% CPU

**Why Alexa Notify Me?**
- Free, no AWS setup
- Works out of box
- Reliable delivery
- Can upgrade to custom skill later

**Why Web Interface?**
- Universal (works on any phone/tablet browser)
- No app store approval needed
- Easy to modify HTML without firmware reflash
- Local network only (secure)

---

**End of Design Specification**
