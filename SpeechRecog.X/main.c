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
#include "speech_data.h"
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * 
 */

//const int8_t PROGMEM mfcc_input[N_MFCC] = {-128, 127, 13, 26, -10, 4, -22, 21, 5, -39, 6, -2, -25};

uint8_t nearest_neighbor(const int8_t* input_mfcc) {
    uint16_t min_dist = 0xFFFF;
    uint8_t best_label = 0;
    for (uint8_t i = 0; i < N_CLASSES; i++) {
        const int8_t* ref_row = (const int8_t*)pgm_read_word(&mfcc_table[i]);
        uint16_t dist = 0;
        for (uint8_t j = 0; j < N_MFCC; j++) {
            int8_t ref = pgm_read_byte(&ref_row[j]);
            int8_t inp = pgm_read_byte(&input_mfcc[j]);
            dist += abs(inp - ref);
        }
        if (dist < min_dist) {
            min_dist = dist;
            best_label = i;
        }
    }
    return best_label;
}

int main(int argc, char** argv) {

    DDRB = 0xff;
    PORTB = 0x00;
    LCD_Init();
    _delay_ms(50);
    LCD_Clear();
    LCD_Gotoxy(0, 0);

    while (1) {

        for (int j = 0; j < N_CLASSES; j++) {
            const int8_t* row = (const int8_t*) pgm_read_word(&mfcc_table[j]);
             uint8_t label = nearest_neighbor(row);
            LCD_Clear();
            LCD_Gotoxy(0, 0);
            LCD_String((char*) labels[label]);
            _delay_ms(50);

            switch (label) {
                case 0:
                    PORTB = 0xff;
                    _delay_ms(1000);
                    break;
                case 1:
                    PORTB = 0x00;
                    _delay_ms(1000);
                    break;
                case 2:
                    PORTB |= (1 << PB0);
                    PORTB &= ~(1 << PB1) & ~(1 << PB2) & ~(1 << PB3);
                    _delay_ms(1000);
                    break;
                case 3:
                    PORTB |= (1 << PB2);
                    PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB3);
                    _delay_ms(1000);
                    break;
                case 4:
                    PORTB |= (1 << PB1);
                    PORTB &= ~(1 << PB0) & ~(1 << PB2) & ~(1 << PB3);
                    _delay_ms(1000);
                    break;
                case 5:
                    PORTB |= (1 << PB3);
                    PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB2);
                    _delay_ms(1000);
                    break;
                case 6:
                    for (int i = 0; i < 4; i++) {
                        PORTB |= (1 << i);
                        _delay_ms(1000);
                    }
                    PORTB = 0x00;
                    _delay_ms(1000);
                    break;
                case 7:
                    PORTB = 0x00;
                    break;
                default:
                    PORTB = 0x00;
                    break;
            }

        }




    }

    return (EXIT_SUCCESS);
}

