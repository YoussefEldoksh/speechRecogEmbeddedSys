/* 
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */
#define F_CPU 11059200UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "speech_data.h"
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ADC_interface.h"
#include "speech_processing.h"

int16_t live_mfcc[N_MFCC];
volatile uint8_t speaking = 0;
//volatile uint8_t counter = 0;

volatile float values_perword[5][N_MFCC];



/*
 * 
 */

//const int8_t PROGMEM mfcc_input[N_MFCC] = {-128, 127, 13, 26, -10, 4, -22, 21, 5, -39, 6, -2, -25};

ISR(INT0_vect) {
    PORTD ^= (1 << PD7);
    speaking = 1;
//    counter += 1;
}

//
//void calculate_training_mean(float values_perword[][N_MFCC], float *out) {
//    for (int i = 0; i < N_MFCC; i++) {
//        float sum = 0;  // ? wider type only here, for the sum
//        for (int j = 0; j < 5; j++) {
//            sum += (float)values_perword[j][i];
//        }
//        out[i] = (float)(sum / 5);  // result fits back in int8_t
//    }
//}
//

int nearest_neighbor(const int16_t* input_mfcc) {
    int32_t min_dist = 0x7FFFFFFF;
    int best_label = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        int16_t ref_row[N_MFCC];
        // copy the row from PROGMEM
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


void init_INT0() {
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    GICR |= (1 << INT0);
    MCUCR |= (1 << ISC01);
    MCUCR &= ~(1 << ISC00);

    GIFR |= (1 << INTF0);
}

int main(int argc, char** argv) {

    DDRB = 0xff;
    PORTB = 0x00;

    ADC_init();
    init_INT0();
    sei();


    LCD_Init();
    _delay_ms(50);

    LCD_Clear();
    LCD_String_xy(0, 0, "Waiting...");

    while (1) {


        // SP_UART_Init(); // Enable only if you need UART debug
        int label = 10;

        _delay_ms(50);

        while (!speaking) {
            if (SP_Is_Speaking()) {
                speaking = 1;
            }
        }

        LCD_Clear();
        LCD_String_xy(0, 0, "Listening...");

        _delay_ms(100);

        SP_Init();
        
        
        SP_Compute_features(live_mfcc);

        //Classification
        label = nearest_neighbor(live_mfcc);
        
//        printf("counter: %d\n", counter);
//        if (counter == 5) {
//            float values[5];
//            calculate_training_mean(values_perword,values);
//            printf("Mean value: %d,%d,%d,%d\n counter: %d\n",values[0],values[1],values[2],values[3], counter);
//            counter = 0;
//        }


        speaking = 0;
        PORTD &= ~(1 << PD7);



                if (label != 10) {
        
                    LCD_Clear();
                    LCD_String_xy(0, 0, (char*) labels[label]);
                    _delay_ms(100);
                    switch (label) {
                        case 0: //on
                            PORTB = 0xff;
                            _delay_ms(1000);
                            break;
                        case 1: //off
                            PORTB = 0x00;
                            _delay_ms(1000);
                            break;
                        case 2: // up
                            PORTB |= (1 << PB0);
                            PORTB &= ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB3);
                            _delay_ms(1000);
                            break;
                        case 3: // down
                            PORTB |= (1 << PB2);
                            PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB3);
                            _delay_ms(1000);
                            break;
                        case 4: // right
                            PORTB |= (1 << PB1);
                            PORTB &= ~(1 << PB0) & ~(1 << PB2) & ~(1 << PB3);
                            _delay_ms(1000);
                            break;
                        case 5: // left
                            PORTB |= (1 << PB3);
                            PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB2);
                            _delay_ms(1000);
                            break;
                        case 6: //start
                            for (int i = 0; i < 4; i++) {
                                PORTB |= (1 << i);
                                _delay_ms(1000);
                            }
                            PORTB = 0x00;
                            _delay_ms(1000);
                            break;
                        case 7: // stop
                            PORTB = 0x00;
                            break;
                        default:
                            PORTB = 0x00;
                            break;
        
        
                    }
        
                }
                
        
        
                label = 10;





    }

    return (EXIT_SUCCESS);
}

