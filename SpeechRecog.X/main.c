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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * 
 */

const int8_t PROGMEM mfcc_input[N_MFCC] = {-128, 127, 17, 39, 6, 10, -26, 10, 31, -15, -15, 4, -11};

uint8_t nearest_neighbor(int8_t* input_mfcc) {
    uint16_t min_dist = 0xFFFF;
    uint8_t best_label = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        uint16_t dist = 0;
        for (uint8_t j = 0; j < N_MFCC; j++) {
            int8_t ref = pgm_read_byte(&mfcc_table[i][j]);
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

    while (1) {
        uint8_t val = nearest_neighbor((int8_t*) mfcc_input);

        switch (val) {
            case 0:
                PORTB = 0xff;
                _delay_ms(1000);
                PORTB = 0x00;
                _delay_ms(1000);
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;
            case 6:
                break;
            case 7:
                break;
            default:
                break;
        }



    }

    return (EXIT_SUCCESS);
}

