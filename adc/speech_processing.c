#include "speech_processing.h"
#include "ADC_interface.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>

volatile uint8_t g_adc_value = 0;
volatile uint8_t  g_sampling_flag = 0;

void Timer0_Init_8kHz(void) {
    TCCR0 |= (1 << WGM01);               // CTC Mode
    TCCR0 |= (1 << CS01) | (1 << CS00);  // Prescaler 64
    OCR0 = 21;                           // Exactly 8kHz for 11.0592MHz clock
    TIMSK |= (1 << OCIE0);               // Enable Interrupt
    sei();
}

ISR(TIMER0_COMP_vect) {
    ADCSRA |= (1 << ADSC);               // Start ADC
    while(ADCSRA & (1 << ADSC));         // Wait for conversion
    g_adc_value = ADCH;                  // 8-bit left-adjusted
    g_sampling_flag = 1;
}

uint8_t is_speaking(void) {
    uint8_t sample = (uint8_t)ADC_u16ReadChannel(0);
    int16_t centered = (int16_t)sample - 128;
    return (abs(centered) > THRESHOLD);
}

static int16_t q15_clamp(int32_t v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

void Capture_MFCC_Lite(int16_t* out) {
    // Compute two short-term features over a fixed 1-second buffer
    int32_t zcr_acc = 0;
    int32_t ste_acc = 0;

    for (uint16_t frame = 0; frame < NUM_FRAMES; frame++) {
        int16_t prev = 0;
        uint16_t crossings = 0;
        uint32_t sum_sq = 0;

        for (uint8_t i = 0; i < FRAME_SIZE; i++) {
            while (!g_sampling_flag);
            g_sampling_flag = 0;

            int16_t sample = (int16_t)g_adc_value - 128; // center 8-bit sample

            if (i > 0) {
                if ((prev < 0 && sample >= 0) || (prev >= 0 && sample < 0)) crossings++;
            }
            prev = sample;

            sum_sq += (uint32_t)(sample * sample);
        }


        // Q15: crossings / FRAME_SIZE (FRAME_SIZE=128 -> *256)
        zcr_acc += (int32_t)crossings * 256;

        // Q15: sum_sq / 64 (for 128-sample frame, 8-bit centered)
        ste_acc += (int32_t)(sum_sq / 64);
    }


    out[0] = q15_clamp(zcr_acc / (int32_t)NUM_FRAMES);
    out[1] = q15_clamp(ste_acc / (int32_t)NUM_FRAMES);
}