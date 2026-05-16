/*
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */


#define F_CPU 11059200UL
#define BAUD_PRESCALE ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)
#define N_CLASSES 8
#define N_VALUES 8
#define OUTTER_ARRAY_SIZE 2

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
float zcr_arr[8];
float ste_arr[8];

const char *labels[N_CLASSES] = {"down",
                                 "left",
                                 //  "off",
                                 //  "on",
                                 "right",
                                 //  "start",
                                 //  "stop",
                                 "up"};

void UART_init(long USAR_BAUDRATE)
{
    UBRRL = BAUD_PRESCALE;
    UBRRH = BAUD_PRESCALE >> 8;
    UCSRB |= (1 << RXEN) | (1 << TXEN);
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
}

uint8_t UART_dataAvailable(void)
{
    return (UCSRA & (1 << RXC));
}

int UART_getChar(FILE *stream)
{
    while ((UCSRA & (1 << RXC)) == 0)
        ;
    return UDR;
}

int UART_putChar(char c, FILE *stream)
{
    while (!(UCSRA & (1 << UDRE)))
        ;
    UDR = c;
    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

ISR(INT0_vect)
{
    recording = !recording;
    PORTD ^= (1 << PD7);
}

ISR(TIMER1_COMPA_vect)
{
    if (!recording)
        return;
    audio_sample = (int8_t)(ADC_read() - 63);
    sample_ready = 1;
}

void Timer1_init()
{
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    OCR1A = (F_CPU / (8UL * 8000UL)) - 1;
    TIMSK |= (1 << OCIE1A);
}

void SPI_init(void)
{
    DDRB |= (1 << PB5) | (1 << PB7) | (1 << PB4);
    DDRB &= ~(1 << PB6);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    SPSR = 0;
}

uint8_t SPI_transfer(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

void CS_LOW(void)
{
    PORTB &= ~(1 << PB4);
}

void CS_HIGH(void)
{
    PORTB |= (1 << PB4);
}

void SRAM_init(void)
{
    CS_LOW();
    SPI_transfer(0x01);
    SPI_transfer(0x40);
    CS_HIGH();
}

void SRAM_write_byte(uint16_t addr, int8_t data)
{
    CS_LOW();
    SPI_transfer(0x02);
    SPI_transfer(addr >> 8);
    SPI_transfer(addr & 0xFF);
    SPI_transfer((uint8_t)data);
    CS_HIGH();
}

int8_t SRAM_read_byte(uint16_t addr)
{
    CS_LOW();
    SPI_transfer(0x03);
    SPI_transfer(addr >> 8);
    SPI_transfer(addr & 0xFF);
    int8_t data = (int8_t)SPI_transfer(0xFF);
    CS_HIGH();
    return data;
}

int nearest_neighbor(float *zcr_in, float *ste_in)
{
    float min_dist = 3.4e+38f;
    int best = 0;

    for (uint8_t i = 0; i < N_CLASSES; i++)
    {
        const float *ref_zcr = (const float *)pgm_read_word(&zcr_table[i]);
        const float *ref_ste = (const float *)pgm_read_word(&ste_table[i]);
        float dist = 0.0f;

        for (uint8_t k = 0; k < N_FRAMES; k++)
        {
            float dz = zcr_in[k] - pgm_read_float(&ref_zcr[k]);
            float ds = ste_in[k] - pgm_read_float(&ref_ste[k]);
            dist += dz * dz + ds * ds;
        }

        printf("%s dist: %ld\n", LABELS[i], (long)dist); // print each distance

        if (dist < min_dist)
        {
            min_dist = dist;
            best = i;
        }
    }
    return best;
}


void init_INT0()
{
    DDRD &= ~(1 << PD2);
    DDRD |= (1 << PD7);
    PORTD |= (1 << PD2);
    PORTD &= ~(1 << PD7);
    GICR |= (1 << INT0);
    MCUCR |= (1 << ISC01);
    GIFR |= (1 << INTF0);
}

void compute_features(uint16_t total_samples, float *feat_zcr, float *feat_ste)
{
    uint16_t window_size = total_samples / 8;

    for (uint8_t i = 0; i < 8; i++)
    {
        float zcr = 0;
        float ste = 0;

        uint16_t start = window_size * i;
        uint16_t end   = window_size * (i + 1);

        int8_t prev = SRAM_read_byte(start);

        for (uint16_t j = start; j < end; j++)
        {
            int8_t sample = SRAM_read_byte(j);

            // ZCR: count sign changes
            if ((prev < 0 && sample > 0) || (prev > 0 && sample < 0))
                zcr += 1.0f;
            prev = sample;

            // STE: sum of squares
            ste += (float)sample * (float)sample;
        }

        feat_zcr[i] = zcr;
        feat_ste[i] = ste;
    }

    // Print for verification
    printf("ZCR: ");
    for (uint8_t i = 0; i < 8; i++) printf("%.2f ", feat_zcr[i]);
    printf("\nSTE: ");
    for (uint8_t i = 0; i < 8; i++) printf("%.2f ", feat_ste[i]);
    printf("\n");
}

void project_Init()
{
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

int main(int argc, char **argv)
{
    project_Init();
    _delay_ms(50);

    while (1)
    {
        if (recording && !prev_recording)
        {
            prev_recording = 1;
            address = 0;
            number_of_samples = 0;
        }
        else if (!recording && prev_recording)
        {
            prev_recording = 0;
            printf("Processing...\n");

            compute_features(number_of_samples, zcr_arr, ste_arr);

            int label = nearest_neighbor(zcr_arr, ste_arr);

            // LCD_Clear();
            // LCD_String_xy(0, 0, (char *) labels[label]);
            printf("%s\n", labels[label]);
            _delay_ms(100);
        }

        if (sample_ready)
        {
            sample_ready = 0;
            if (number_of_samples < 12000)
            {
                SRAM_write_byte(address, audio_sample);
                address++;
                number_of_samples++;
            }
        }
    }
    return EXIT_SUCCESS;
}