/*
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */
#define F_CPU 11059200UL
#define BAUD_PRESCALE ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)
#define FFT_SIZE 64
#define FFT_LOG2 6
#define N_BINS 32
#define HOP_SIZE 32
#define N_BANDS 6
#define ADC_CENTER 128
#define TIMER_TICK_HZ 8000
#define BUTTON_DEBOUNCE_MS 20
#define FFT_INPUT_SHIFT 7
#define SILENCE_THRESHOLD 0
#define PRE_EMPHASIS_NUM 95

#include "fix_fft.h"
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
static int16_t prev_sample = 0;

/* ?? Mel band bin boundaries (linear spacing on 0..N_BINS-1, 6 bands) ?? */
static const uint8_t band_start[N_BANDS] PROGMEM = {1, 2, 4, 8, 16, 24};
static const uint8_t band_end [N_BANDS] PROGMEM = {2, 4, 8, 16, 24, 32};

/* ------------------------------------------------------------------ */
/*  Integer log2 approximation                                         */
/*  Returns 8 * log2(x+1) as a uint16_t  (avoids float, avoids libm)  */
/*  Range: x up to ~65535 ? result up to ~128                         */

/* ------------------------------------------------------------------ */
static uint16_t ilog2_scaled(uint32_t x) {
    if (x == 0) return 0;
    uint8_t shift = 0;
    uint32_t v = x;
    while (v > 1) {
        v >>= 1;
        shift++;
    } // integer part of log2
    // fractional part: (x >> (shift-3)) & 0x07  gives 3 extra bits
    uint16_t frac = (shift >= 3) ? ((x >> (shift - 3)) & 0x07) : 0;
    return (uint16_t) ((uint16_t) shift * 8 + frac); // 8 * log2(x), approx
}

static int8_t clamp_int8(int16_t v) {
    if (v > 127) return 127;
    if (v < -128) return -128;
    return (int8_t) v;
}

static void get_label(uint8_t idx, char *out, size_t out_size) {
    if (out_size == 0) return;
    if (idx >= N_WORDS) {
        out[0] = '\0';
        return;
    }
    PGM_P ptr = (PGM_P) pgm_read_ptr(&LABELS[idx]);
    strncpy_P(out, ptr, out_size - 1);
    out[out_size - 1] = '\0';
}

/* ================================================================== */

void UART_init(long USAR_BAUDRATE) {
    UBRRL = BAUD_PRESCALE;
    UBRRH = BAUD_PRESCALE >> 8;
    UCSRB |= (1 << RXEN) | (1 << TXEN);
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
}

uint8_t UART_dataAvailable(void) {
    return (UCSRA & (1 << RXC));
}

int UART_getChar(FILE *stream) {
    while ((UCSRA & (1 << RXC)) == 0);
    return UDR;
}

int UART_putChar(char c, FILE *stream) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = c;
    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

ISR(INT0_vect) {
    _delay_ms(BUTTON_DEBOUNCE_MS);
    if (!(PIND & (1 << PD2))) {
        recording = !recording;
        PORTD ^= (1 << PD7);
    }
}

ISR(TIMER1_COMPA_vect) {
    if (!recording) return;
    int16_t raw_val = ADC_read();
    int16_t centered = raw_val - ADC_CENTER;
    int16_t pre_emph = centered - (int16_t) ((PRE_EMPHASIS_NUM * prev_sample) / 100);
    prev_sample = centered;
    audio_sample = clamp_int8(pre_emph);
    sample_ready = 1;
}

void Timer1_init() {
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    OCR1A = (F_CPU / (8UL * TIMER_TICK_HZ)) - 1;
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
    while (!(SPSR & (1 << SPIF)));
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
    SPI_transfer(0x02);
    SPI_transfer(addr >> 8);
    SPI_transfer(addr & 0xFF);
    SPI_transfer((uint8_t) data);
    CS_HIGH();
}

int8_t SRAM_read_byte(uint16_t addr) {
    CS_LOW();
    SPI_transfer(0x03);
    SPI_transfer(addr >> 8);
    SPI_transfer(addr & 0xFF);
    int8_t data = (int8_t) SPI_transfer(0xFF);
    CS_HIGH();
    return data;
}

/* ?? Nearest-neighbour (unchanged logic, works for any N_FEATURES) ?? */
int nearest_neighbor(uint16_t *input) {
    uint32_t min_dist = 0xFFFFFFFF;
    int best = 0;

    for (uint8_t i = 0; i < N_WORDS; i++) {
        const uint16_t *ref = (const uint16_t *) pgm_read_ptr(&feat_table[i]);
        uint32_t dist = 0;
        for (uint8_t k = 0; k < N_FEATURES; k++) {
            int32_t diff = (int32_t) input[k] - pgm_read_word(&ref[k]);
            dist += (uint32_t) (diff * diff);
        }
        if (dist < min_dist) {
            min_dist = dist;
            best = i;
        }
    }
    return best;
}

static void dump_distances(uint16_t *input) {
    char label_buf[12];
    for (uint8_t i = 0; i < N_WORDS; i++) {
        const uint16_t *ref = (const uint16_t *) pgm_read_ptr(&feat_table[i]);
        uint32_t dist = 0;
        for (uint8_t k = 0; k < N_FEATURES; k++) {
            int32_t diff = (int32_t) input[k] - pgm_read_word(&ref[k]);
            dist += (uint32_t) (diff * diff);
        }
        get_label(i, label_buf, sizeof(label_buf));
        printf("dist %s: %lu\n", label_buf, dist);
    }
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

void pre_emphasis(short *x, uint8_t len) {
    if (len < 2) return;
    for (uint8_t i = len - 1; i > 0; i--)
        x[i] = x[i] - (short) (0.97f * x[i - 1]);
    x[0] = 0;
}
//
//void apply_hamming_window(short *real) {
//    for (uint8_t i = 0; i < FFT_SIZE; i++) {
//        uint8_t w = pgm_read_byte(&hamming_window[i]);
//        real[i] = (short)(((int16_t)real[i] * w) >> 8);
//    }
//}

/* ================================================================== */
/*  compute_features                                                   */
/*                                                                     */
/*  Feature vector layout (matches Python pipeline):                   */
/*    feat_out[0]   = ZCR mean        (counts per frame, scaled *4)  */
/*    feat_out[1]   = log-STE mean    (ilog2_scaled of frame energy)  */
/*    feat_out[2..7]= log Mel band energies [band 0 .. band 5]        */

/* ================================================================== */
void compute_features(uint16_t total_samples, uint16_t *feat_out) {

    uint16_t n_frames = 0;
    uint16_t addr = 0;

    uint32_t accum_zcr = 0;
    uint32_t accum_ste = 0;
    uint32_t accum_mel[N_BANDS];
    for (uint8_t b = 0; b < N_BANDS; b++) accum_mel[b] = 0;

    // ?? single reusable frame buffer (int16, 128 bytes on stack) ??
    int16_t frame_buf[FFT_SIZE];

    // printf("Total Samples: %u\n", total_samples);

    while (addr + FFT_SIZE <= total_samples) {

        /* ?? 1. Read frame ONCE from SRAM ?? */
        for (uint8_t i = 0; i < FFT_SIZE; i++) {
            frame_buf[i] = (int16_t) SRAM_read_byte(addr + i);
            //            printf("Address: %d | data: %d\n", addr + i, frame_buf[i]);
        }

        // printf("after reading data \n");

        /* ?? 2. Short-Time Energy + VAD on raw samples ?? */
        uint32_t frame_energy = 0;
        for (uint8_t i = 0; i < FFT_SIZE; i++) {
            int16_t s = frame_buf[i];
            frame_energy += (uint32_t) (s * s);
        }
        if (frame_energy < SILENCE_THRESHOLD) {
            addr += HOP_SIZE;
            continue;
        }

        /* ?? 3. ZCR on raw int16 samples ?? */
        uint8_t zcr_count = 0;
        for (uint8_t i = 1; i < FFT_SIZE; i++) {
            if ((frame_buf[i] >= 0) != (frame_buf[i - 1] >= 0))
                zcr_count++;
        }
        accum_zcr += (uint16_t) zcr_count * 4u; // scale: (count/64)*256 = count*4

        accum_ste += ilog2_scaled(frame_energy);


        /* ?? 4. Load into FFT buffer, pre-emphasis, FFT ?? */
        for (uint8_t i = 0; i < FFT_SIZE; i++) {
            fft_real[i] = (short) frame_buf[i] << FFT_INPUT_SHIFT;
            fft_imag[i] = 0;
        }


        fix_fft(fft_real, fft_imag, FFT_LOG2, 0);
        
        // printf("after fix_fft\n ");



        /* ?? 5. Mel band log-energies ?? */
        for (uint8_t b = 0; b < N_BANDS; b++) {
            uint8_t k0 = pgm_read_byte(&band_start[b]);
            uint8_t k1 = pgm_read_byte(&band_end[b]);
            uint32_t band_energy = 0;

            for (uint8_t k = k0; k < k1; k++) {
                int16_t re = fft_real[k];
                int16_t im = fft_imag[k];
                band_energy += (uint32_t) ((int32_t) re * re + (int32_t) im * im);
            }
            accum_mel[b] += ilog2_scaled(band_energy);
        }



        n_frames++;
        addr += HOP_SIZE;
    }

    // printf("n_frames: %u\n", n_frames);

    if (n_frames > 0) {
        feat_out[0] = (uint16_t) (accum_zcr / n_frames);
        feat_out[1] = (uint16_t) (accum_ste / n_frames);
        for (uint8_t b = 0; b < N_BANDS; b++)
            feat_out[b + 2] = (uint16_t) (accum_mel[b] / n_frames);
    } else {
        for (uint8_t k = 0; k < N_FEATURES; k++)
            feat_out[k] = 0;
    }

        printf("feat: %u %u %u %u %u %u %u %u\n",
            feat_out[0], feat_out[1], feat_out[2], feat_out[3],
            feat_out[4], feat_out[5], feat_out[6], feat_out[7]);
}

/* ================================================================== */

void project_Init() {
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

            compute_features(number_of_samples, features);

            dump_distances(features);

            int label = nearest_neighbor(features);

            // LCD_Clear();
            // LCD_String_xy(0, 0, (char *) labels[label]);
            char label_buf[12];
            get_label((uint8_t) label, label_buf, sizeof(label_buf));
            printf("%s\n", label_buf);
            _delay_ms(100);
        }

        if (sample_ready) {
            sample_ready = 0;
            if (number_of_samples < 12000) {
                SRAM_write_byte(address, audio_sample);
                address++;
                number_of_samples++;
            }
        }
    }
    return EXIT_SUCCESS;
}