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
