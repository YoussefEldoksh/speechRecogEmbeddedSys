#ifndef SPEECH_PROCESSING_H_
#define SPEECH_PROCESSING_H_

#include <avr/io.h>

#define F_CPU 11059200UL
#define THRESHOLD      80     // Adjust based on mic sensitivity
#define SAMPLE_COUNT   8000   // 1 second of audio
#define FRAME_SIZE     128    
#define NUM_FRAMES     (SAMPLE_COUNT / FRAME_SIZE)
#define N_MFCC         13     // Must match speech_data.h

// Global flags for the Timer ISR
extern volatile uint16_t g_adc_value;
extern volatile uint8_t  g_sampling_flag;

void Timer0_Init_8kHz(void);
uint8_t is_speaking(void);
void Capture_MFCC_Lite(int8_t* live_features);

#endif