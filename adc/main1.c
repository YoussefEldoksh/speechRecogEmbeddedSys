#define F_CPU 11059200UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>

#include "speech_data.h"
#include "lcd.h"
#include "ADC_interface.h"
#include "speech_processing.h"

// Buffer for real-time captured features (Q15: zcr, ste)
int16_t live_mfcc[N_MFCC];

uint8_t nearest_neighbor(int16_t* input_mfcc) {
    int32_t min_dist = 0x7FFFFFFF;
    uint8_t best_label = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        int16_t ref_row[N_MFCC];
        memcpy_P(ref_row, &features_table[i][0], sizeof(ref_row));

        int32_t dist = 0;
        for (uint8_t j = 0; j < N_MFCC; j++) {
            int16_t d = input_mfcc[j] - ref_row[j];
            if (d < 0) d = (int16_t)-d;
            dist += d;
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