/*
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */
#define F_CPU 11059200UL
#define BAUD_PRESCALE ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)
#define N_CLASSES 8
#define FFT_SIZE 64
#define FFT_LOG2 6
#define N_BINS 32
#define HOP_SIZE 32

#include "fix_fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "speech_data.h"
#include "lcd.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ADC_interface.h"
#include <math.h>

volatile uint8_t recording = 0;
volatile int8_t prev_recording = 0;
volatile int8_t audio_sample = 0;
volatile uint8_t sample_ready = 0;
volatile int number_of_samples = 0;
volatile uint16_t address = 0;
uint16_t features[N_FEATURES];
short fft_real[FFT_SIZE];
short fft_imag[FFT_SIZE];
const char *labels[N_CLASSES] = {"on", "off", "up", "down", "right", "left", "start", "stop"};

/*
 *
 */

void UART_init(long USAR_BAUDRATE) {
    UBRRL = BAUD_PRESCALE;

    UBRRH = (BAUD_PRESCALE >> 8);

    UCSRB |= (1 << RXEN) | (1 << TXEN);

    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
}

uint8_t UART_dataAvailable(void) {
    return (UCSRA & (1 << RXC));
}

int UART_getChar(FILE *stream) {
    while ((UCSRA & (1 << RXC)) == 0)
        ;

    return UDR;
}

int UART_putChar(char c, FILE *stream) {
    while (!(UCSRA & (1 << UDRE)))
        ;

    UDR = c;

    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

ISR(INT0_vect) {
    recording = !recording;
    PORTD ^= (1 << PD7);
}

ISR(TIMER1_COMPA_vect) {
    if (!recording)
        return;

    audio_sample = (int8_t) (ADC_read() - 128);
    sample_ready = 1;
}

void Timer1_init() {
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS11); // prescaler 8

    OCR1A = (F_CPU / (8UL * 8000UL)) - 1; // 8kHz

    TIMSK |= (1 << OCIE1A);
}

void SPI_init(void) {
    DDRB |= (1 << PB5) | (1 << PB7) | (1 << PB4);
    DDRB &= ~(1 << PB6);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    SPSR = 0;
}

uint8_t SPI_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

void CS_LOW(void) {
    PORTB &= ~(1 << PB4);
}

void CS_HIGH(void) {
    PORTB |= (1 << PB4);
}

void SRAM_init(void) {
    CS_LOW();
    SPI_transfer(0x01);
    SPI_transfer(0x40);
    CS_HIGH();
}

void SRAM_write_byte(uint16_t addr, int8_t data) {
    CS_LOW();
    SPI_transfer(0x02); // Write command
    SPI_transfer(addr >> 8); // High byte of address
    SPI_transfer(addr & 0xFF); // Low byte of address
    SPI_transfer((uint8_t) data); // Send 8-bit signed data
    CS_HIGH();
}

int8_t SRAM_read_byte(uint16_t addr) {
    CS_LOW();
    SPI_transfer(0x03); // Read command
    SPI_transfer(addr >> 8);
    SPI_transfer(addr & 0xFF);

    int8_t data = (int8_t) SPI_transfer(0xFF); // Read 8-bit data

    CS_HIGH();
    return data;
}

int nearest_neighbor(uint16_t *input) {
    uint32_t min_dist = 0xFFFFFFFF;
    int best = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++) {
        const uint16_t *ref = (const uint16_t *) pgm_read_word(&feat_table[i]);
        uint32_t dist = 0;
        for (uint8_t k = 0; k < N_FEATURES; k++) {
            int32_t diff = (int32_t) input[k] - pgm_read_word(&ref[k]);
            dist += (uint32_t) (diff * diff >> 8); // shift to prevent overflow
        }
        if (dist < min_dist) {
            min_dist = dist;
            best = i;
        }
    }
    return best;
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

// Pre-emphasis filter

void pre_emphasis(short *x, uint8_t len) {
    if (len < 2)
        return;
    for (uint8_t i = len - 1; i > 0; i--) {
        x[i] = x[i] - (short) (PRE_EMPHASIS * x[i - 1]);
    }
    x[0] = 0;
}

// Apply Hamming window from PROGMEM

void apply_hamming_window(short *real) {
    for (uint8_t i = 0; i < FFT_SIZE; i++) {
        uint8_t w = pgm_read_byte(&hamming_window[i]);
        real[i] = (short) (((int16_t) real[i] * w) >> 8);
    }
}


static const uint8_t band_start[6] PROGMEM = {0, 2, 4, 8, 16, 24};
static const uint8_t band_end[6] PROGMEM = {2, 4, 8, 16, 24, 32};
#define N_BANDS 6

void compute_features(uint16_t total_samples, uint16_t *feat_out) {
    uint16_t n_frames = 0;
    uint16_t addr = 0;
    uint32_t accum[8] = {0};
    uint32_t band_sum[N_BANDS];

    printf("Total Samples: %u\n", total_samples);

    while (addr + FFT_SIZE <= total_samples) {

        for (uint8_t b = 0; b < N_BANDS; b++) band_sum[b] = 0;

        for (uint8_t i = 0; i < FFT_SIZE; i++) {
            fft_real[i] = (short) SRAM_read_byte(addr + i) << 6;
            fft_imag[i] = 0;
        }

        pre_emphasis(fft_real, FFT_SIZE);
        apply_hamming_window(fft_real);
        fix_fft(fft_real, fft_imag, FFT_LOG2, 0);

        uint32_t total_energy = 0;
        uint32_t weighted = 0;

        for (uint8_t k = 0; k < N_BINS; k++) {
            uint16_t m = (uint16_t) (abs(fft_real[k]) + abs(fft_imag[k]));

            if (k < 2) band_sum[0] += m;
            else if (k < 4) band_sum[1] += m;
            else if (k < 8) band_sum[2] += m;
            else if (k < 16) band_sum[3] += m;
            else if (k < 24) band_sum[4] += m;
            else band_sum[5] += m;

            if (k == 0) continue;
            total_energy += m;
            weighted += (uint32_t) k * m;
        }

        accum[0] += band_sum[0] >> 1;
        accum[1] += band_sum[1] >> 1;
        accum[2] += band_sum[2] >> 2;
        accum[3] += band_sum[3] >> 3;
        accum[4] += band_sum[4] >> 3;
        accum[5] += band_sum[5] >> 3;
        accum[6] += (total_energy >> 6);
        accum[7] += (total_energy > 0) ? (uint16_t) (weighted / total_energy) : 0;

        n_frames++;
        addr += HOP_SIZE;
    }

    printf("n_frames: %u\n", n_frames);

    // single printf for all features ? one call, minimal stack
    printf("feat: %u %u %u %u %u %u %u %u\n",
            (unsigned) (n_frames > 0 ? accum[0] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[1] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[2] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[3] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[4] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[5] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[6] / n_frames : 0),
            (unsigned) (n_frames > 0 ? accum[7] / n_frames : 0));

    if (n_frames > 0)
        for (uint8_t k = 0; k < N_FEATURES; k++)
            feat_out[k] = (uint16_t) (accum[k] / n_frames);
    else
        for (uint8_t k = 0; k < N_FEATURES; k++)
            feat_out[k] = 0;
}

void project_Init() {
    //    DDRB = 0xFF;
    //    PORTB = 0xFF;
    UART_init(9600);
    stdout = &uart_str;

    SPI_init();

    SRAM_init();

    Timer1_init();

    ADC_init();

    init_INT0();

    LCD_Init();
    sei();
}

int main(int argc, char **argv) {

    project_Init();
    _delay_ms(50);

    while (1) {
        if (recording && !prev_recording) {
            prev_recording = 1;
            address = 0;
            number_of_samples = 0;
        } else if (!recording && prev_recording) {
            prev_recording = 0;

            printf("Processing...\n");

            //            LCD_Clear();
            //            LCD_String_xy(0, 0, "Processing...");

            for (int i = 0; i < 10; i++) {
                printf("Values in sram: address: %d -> %d\n",i, SRAM_read_byte(i));

            }

            printf("Values in sram: address: %d -> %d");

            compute_features(number_of_samples, features);

            int label = nearest_neighbor(features);

            LCD_Clear();
            LCD_String_xy(0, 0, (char *) labels[label]);
            printf("%s", (char *) labels[label]);
            _delay_ms(100);
        }

        if (sample_ready) {
            sample_ready = 0;
            if (number_of_samples < 12000) { // safety limit (~1 second)
                SRAM_write_byte(address, audio_sample);
                address++;
                number_of_samples++;
            }
        }
    }
    return (EXIT_SUCCESS);
}
