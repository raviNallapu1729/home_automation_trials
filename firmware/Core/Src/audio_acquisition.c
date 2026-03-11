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
