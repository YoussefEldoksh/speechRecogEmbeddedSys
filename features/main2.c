#include <avr/io.h>
#include <avr/interrupt.h>
#include "features.h"
#include "speech_data.h"

//external SRAM functions:
// void sram_write(uint16_t address, uint8_t data);
// uint8_t sram_read(uint16_t address);

void process_recognition() {
    FeatureVector live_features;
    uint32_t word_totals[13] = {0};

    // 1. Average features across the recorded SRAM data
    for (uint16_t i = 0; i < 8000; i += FRAME_SIZE) {
        uint8_t frame[FRAME_SIZE];
        for(int j=0; j<FRAME_SIZE; j++) frame[j] = sram_read(i + j);
        
        FeatureVector temp = extract_all_features(frame);
        for(int k=0; k<13; k++) word_totals[k] += temp.vals[k];
    }

    for(int k=0; k<13; k++) live_features.vals[k] = word_totals[k] / (8000/FRAME_SIZE);

    // 2. Compare to PROGMEM templates
    uint8_t best_match = 0;
    uint32_t min_error = 0xFFFFFFFF;

    for (uint8_t w = 0; w < 8; w++) {
        uint32_t current_error = 0;
        for (uint8_t f = 0; f < 13; f++) {
            uint8_t t_val = pgm_read_byte(&word_templates[w][f]);
            current_error += ABS_DIFF(live_features.vals[f], t_val);
        }

        if (current_error < min_error) {
            min_error = current_error;
            best_match = w;
        }
    }

    // 3. Act: Trigger LED
    PORTB = (1 << best_match);
}

int main() {
    // Init ADC, Timer, SRAM, and Ports here
    while(1) {
        // Wait for trigger -> Record to SRAM -> process_recognition()
    }
}