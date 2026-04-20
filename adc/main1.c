#define F_CPU 11059200UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "speech_data.h"
#include "lcd.h"
#include "ADC_interface.h"
#include "speech_processing.h"

// Buffer for real-time captured features
int8_t live_mfcc[N_MFCC];

uint8_t nearest_neighbor(int8_t* input_mfcc) {
    uint16_t min_dist = 0xFFFF;
    uint8_t best_label = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        // Read pointer to the template row from Flash
        const int8_t* ref_row = (const int8_t*)pgm_read_word(&mfcc_table[i]);
        uint16_t dist = 0;

        for (uint8_t j = 0; j < N_MFCC; j++) {
            int8_t ref = pgm_read_byte(&ref_row[j]); // From Flash
            int8_t inp = input_mfcc[j];              // From RAM
            dist += abs(inp - ref);
        }

        if (dist < min_dist) {
            min_dist = dist;
            best_label = i;
        }
    }
    return best_label;
}

void handle_outputs(uint8_t label) {
    LCD_Clear();
    LCD_Gotoxy(0, 0);
    // Print label string from Flash
    LCD_String((char*)labels[label]);

    // Simple LED mapping
    if (label == 0) PORTB = 0xFF;      // "ON"
    else if (label == 1) PORTB = 0x00; // "OFF"
    else PORTB = (1 << label);         // Others
}

int main(void) {
    //Initializations
    DDRB = 0xFF; 
    PORTB = 0x00;
    ADC_init();
    LCD_Init();
    Timer0_Init_8kHz();

    LCD_String("Voice System Ready");
    _delay_ms(1000);
    LCD_Clear();

    while (1) {
        LCD_Gotoxy(0,0);
        LCD_String("Waiting...");

        //VAD - Wait for speech
        if (is_speaking()) {
            LCD_Clear();
            LCD_String("Listening...");
            
            //Feature Extraction
            Capture_MFCC_Lite(live_mfcc);

            //Classification
            uint8_t word_index = nearest_neighbor(live_mfcc);

            //Output
            handle_outputs(word_index);

            _delay_ms(1500);
            LCD_Clear();
        }
    }
    return 0;
}