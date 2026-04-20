#include "speech_processing.h"
#include "ADC_interface.h"
#include <avr/interrupt.h>
#include <stdlib.h>

volatile uint16_t g_adc_value = 0;
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
    g_adc_value = ADC;
    g_sampling_flag = 1;
}

uint8_t is_speaking(void) {
    uint16_t sample = ADC_u16ReadChannel(0);
    int16_t centered = (int16_t)sample - 512;
    return (abs(centered) > THRESHOLD);
}

void Capture_MFCC_Lite(int8_t* live_features) {
    // We divide the 1-second recording into 13 segments 
    // to fill the 13 MFCC slots required by speech_data.h
    uint16_t segments = SAMPLE_COUNT / N_MFCC;
    
    for (int i = 0; i < N_MFCC; i++) {
        uint32_t segment_energy = 0;
        for (uint16_t j = 0; j < segments; j++) {
            while(!g_sampling_flag);
            g_sampling_flag = 0;
            
            int16_t val = (int16_t)g_adc_value - 512;
            segment_energy += (uint32_t)abs(val);
        }
        // Normalize the energy to fit in an int8_t (-128 to 127)
        // Adjust the shift (>> 4) based on your Python training scaling
        int32_t normalized = (segment_energy / segments) - 128;
        if (normalized > 127) normalized = 127;
        if (normalized < -128) normalized = -128;
        live_features[i] = (int8_t)normalized;
    }
}