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
