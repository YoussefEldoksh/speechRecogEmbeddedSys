/* 
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */
#define F_CPU 11059200UL
#define BAUD_PRESCALE  ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "speech_data.h"
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ADC_interface.h"
#include "speech_processing.h"
#include <math.h>

float live_mfcc[N_MFCC];
volatile uint8_t recording = 0;
volatile int8_t prev_recording = 0;
volatile int8_t audio_sample = 0;
volatile uint8_t sample_ready = 0;
volatile int number_of_samples = 0;



/*
 * 
 */

//const int8_t PROGMEM mfcc_input[N_MFCC] = {-128, 127, 13, 26, -10, 4, -22, 21, 5, -39, 6, -2, -25};
const char* labels[N_CLASSES] = {"on", "off", "up", "down", "right", "left", "start", "stop"};

ISR(INT0_vect) {
    recording = !recording;
    PORTD ^= (1 << PD7);
}

ISR(TIMER1_COMPA_vect) {
    if (!recording) return;

    audio_sample = (int8_t) (ADC_read() - 128);
    ;
    sample_ready = 1;
}




// ================= TIMER1 (8kHz) =================

void Timer1_init() {
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS11); // prescaler 8

    OCR1A = (F_CPU / (8UL * 8000UL)) - 1; // 8kHz

    TIMSK |= (1 << OCIE1A);
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

int nearest_neighbor(const float* input_mfcc) {
    float min_dist = 100000000;
    int best_label = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        const float* ref_row = (const float*) pgm_read_word(&features_table[i]);
        float f0 = pgm_read_float(&ref_row[0]);
        float f1 = pgm_read_float(&ref_row[1]);
        printf("class %d: %f %f\n", i, f0, f1);
    }

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        const float* ref_row = (const float*) pgm_read_word(&features_table[i]);
        float dist_squared = 0;

        for (uint8_t j = 0; j < N_MFCC; j++) {
            float ref = pgm_read_float(&ref_row[j]);
            float inp = pgm_read_float(&input_mfcc[j]);
            float diff = inp - ref;
            dist_squared += pow(diff, 2); // Sum of squares
        }

        float dist = sqrt(dist_squared);

        if (dist < min_dist) {
            min_dist = dist;
            best_label = i;
        }
    }

    return best_label;
}

void init_INT0() {
    DDRD &= ~(1 << PD2);
    DDRD |= (1 << PD7);
    PORTD |= (1 << PD2);
    PORTD &= ~(1 << PD7);

    GICR |= (1 << INT0);
    MCUCR |= (1 << ISC01);
    GIFR |= (1 << INTF0);
}

void do_task(int label) {

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
}

int main(int argc, char** argv) {

    DDRB = 0xff;
    PORTB = 0x00;

    float zcr = 0.0f;
    float ste = 0.0f;
    int8_t adc_new_value = 0;
    int8_t adc_prev_value = 0;
    Timer1_init();
    SP_UART_Init();

    ADC_init();
    init_INT0();
    sei();


    LCD_Init();
    _delay_ms(50);

    int label = 10;

    while (1) {
        //        printf("alive\n");
        //        _delay_ms(500);

        // START / STOP messages
        if (recording && !prev_recording) {

            zcr = 0.0f;
            ste = 0.0f;
            number_of_samples = 0;
            prev_recording = 1;
        } else if (!recording && prev_recording) {
            prev_recording = 0;
            _delay_ms(50);

            // classify HERE, after accumulation is done
            live_mfcc[0] = zcr / number_of_samples;
            live_mfcc[1] = (ste / number_of_samples) / 100000 ;
            label = nearest_neighbor(live_mfcc);
            do_task(label);

            printf("zcr: %f , ste: %f\n", live_mfcc[0], live_mfcc[1]);
            ste = 0.0f;

            zcr = 0.0f;
            ste = 0.0f;

        }
        if (sample_ready) {
            adc_new_value = audio_sample;
            number_of_samples++;
            if ((adc_prev_value < 0 && adc_new_value >= 0) || (adc_prev_value >= 0 && adc_new_value < 0)) {
                zcr++;
            }

            ste += pow(adc_new_value, 2);

//            printf("ADC: %d\n", adc_new_value);

            adc_prev_value = adc_new_value;

            sample_ready = 0;

        }
    }

    return (EXIT_SUCCESS);
}

