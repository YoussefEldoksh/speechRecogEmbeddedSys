/*
 * File:   main.c
 * Author: Lenovo
 *
 * Created on April 18, 2026, 7:40 PM
 */

#define F_CPU 11059200UL
#define BAUD_PRESCALE ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)

#include "ADC_interface.h"
#include "lcd.h"
#include "speech_data.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>


volatile uint8_t recording = 0;
volatile int8_t prev_recording = 0;
volatile uint8_t audio_sample_raw = 0; /* raw 0-255 from ADC (uint8_t) */
volatile uint8_t sample_ready = 0;
volatile uint8_t button_event = 0;
volatile uint16_t number_of_samples = 0;
volatile uint16_t address = 0;

float zcr_arr[N_FRAMES];
float ste_arr[N_FRAMES];
/* Store label strings in RAM (fixed-width) so LCD_String can read them reliably */
char labels[N_CLASSES][8] = {"back", "go", "one", "ship", "acht"};

/* ------------------------------------------------------------------ */
/*  UART                                                               */
/* ------------------------------------------------------------------ */
void UART_init(long USAR_BAUDRATE) {
  UBRRL = BAUD_PRESCALE;
  UBRRH = BAUD_PRESCALE >> 8;
  UCSRB |= (1 << RXEN) | (1 << TXEN);
  UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
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

static FILE uart_str =
    FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

/* ------------------------------------------------------------------ */
/*  Interrupts                                                         */
/* ------------------------------------------------------------------ */
ISR(INT0_vect) { button_event = 1; }

ISR(TIMER1_COMPA_vect) {
  if (!recording)
    return;
  /*
   * Store raw unsigned byte (0-255) exactly as Python receives it
   * over UART.  Subtraction to signed domain happens at feature time.
   */
  audio_sample_raw = (uint8_t)ADC_read();
  sample_ready = 1;
}

/* ------------------------------------------------------------------ */
/*  Timer1 – 8 kHz CTC                                                */
/* ------------------------------------------------------------------ */
void Timer1_init(void) {
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);
  OCR1A = (F_CPU / (8UL * 8000UL)) - 1;
  TIMSK |= (1 << OCIE1A);
}

/* ------------------------------------------------------------------ */
/*  SPI / SRAM                                                         */
/* ------------------------------------------------------------------ */
void SPI_init(void) {
  DDRB |= (1 << PB5) | (1 << PB7) | (1 << PB4) | (1 << PB0) | (1 << PB1) |
          (1 << PB2);
  DDRB &= ~(1 << PB6);
  PORTB &= ~(1 << PB0) & ~(1 << PB1) & ~(1 << PB2);
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
  SPSR = 0;
}

uint8_t SPI_transfer(uint8_t data) {
  SPDR = data;
  while (!(SPSR & (1 << SPIF)))
    ;
  return SPDR;
}

void CS_LOW(void) { PORTB &= ~(1 << PB4); }
void CS_HIGH(void) { PORTB |= (1 << PB4); }

void SRAM_init(void) {
  CS_LOW();
  SPI_transfer(0x01);
  SPI_transfer(0x40);
  CS_HIGH();
}

/* Store raw unsigned byte */
void SRAM_write_byte(uint16_t addr, uint8_t data) {
  CS_LOW();
  SPI_transfer(0x02);
  SPI_transfer(addr >> 8);
  SPI_transfer(addr & 0xFF);
  SPI_transfer(data);
  CS_HIGH();
}

/* Return raw unsigned byte */
uint8_t SRAM_read_byte(uint16_t addr) {
  CS_LOW();
  SPI_transfer(0x03);
  SPI_transfer(addr >> 8);
  SPI_transfer(addr & 0xFF);
  uint8_t data = SPI_transfer(0xFF);
  CS_HIGH();
  return data;
}

/* ------------------------------------------------------------------ */
/*  INT0                                                               */
/* ------------------------------------------------------------------ */
void init_INT0(void) {
  DDRD &= ~(1 << PD2);
  DDRD |= (1 << PD7);
  PORTD |= (1 << PD2);
  PORTD &= ~(1 << PD7);
  GICR |= (1 << INT0);
  MCUCR |= (1 << ISC01);
  GIFR |= (1 << INTF0);
}

/* ------------------------------------------------------------------ */
/*  Feature extraction  – mirrors Python exactly                       */
/*                                                                     */
/*  Python stores raw samples (0-255).                                 */
/*  It shifts to signed via:  s = sample - 128                        */
/*  ZCR threshold:            zero-crossing at 0 (after shift)        */
/*  STE:                      f = s / 128.0 ; ste += f*f              */
/*  Normalisation:            per-feature min-max over the 8 frames   */
/* ------------------------------------------------------------------ */
void compute_features(uint16_t total_samples, float *feat_zcr,
                      float *feat_ste) {
  int window_size = total_samples / N_FRAMES;

  for (uint8_t i = 0; i < N_FRAMES; i++) {
    float zcr = 0.0f;
    float ste = 0.0f;

    int start = window_size * i;
    int end = window_size * (i + 1);

    /* shift raw byte to signed domain, same as Python: s = sample - 128 */
    uint8_t prev = SRAM_read_byte(start);

    for (int j = start; j < end; j++) {
      uint8_t s = SRAM_read_byte(j);

      /* zero-crossing at 0 – matches Python after shift */
      if ((prev < 60 && s >= 60) || (prev >= 60 && s < 60))
        zcr += 1.0f;
      prev = s;

      /* STE: f = s/128.0, ste += f*f – identical to Python */
      float f = (float)s / 128.00f;
      ste += f * f;
    }

    feat_zcr[i] = zcr;
    feat_ste[i] = ste;
  }

  /* --- min-max normalisation over the 8 frames (same as Python) --- */

  float min_zcr = feat_zcr[0], max_zcr = feat_zcr[0];
  for (uint8_t i = 1; i < N_FRAMES; i++) {
    if (feat_zcr[i] < min_zcr)
      min_zcr = feat_zcr[i];
    if (feat_zcr[i] > max_zcr)
      max_zcr = feat_zcr[i];
  }
  float zcr_range = max_zcr - min_zcr;
  for (uint8_t i = 0; i < N_FRAMES; i++)
    feat_zcr[i] =
        (zcr_range == 0.000f) ? 0.000f : (feat_zcr[i] - min_zcr) / zcr_range;

  float min_ste = feat_ste[0], max_ste = feat_ste[0];
  for (uint8_t i = 1; i < N_FRAMES; i++) {
    if (feat_ste[i] < min_ste)
      min_ste = feat_ste[i];
    if (feat_ste[i] > max_ste)
      max_ste = feat_ste[i];
  }
  float ste_range = max_ste - min_ste;
  for (uint8_t i = 0; i < N_FRAMES; i++)
    feat_ste[i] =
        (ste_range == 0.000f) ? 0.000f : (feat_ste[i] - min_ste) / ste_range;

  /* --- debug print --- */
  printf("ZCR: ");
  for (uint8_t i = 0; i < N_FRAMES; i++)
    printf("%.3f ", feat_zcr[i]);
  printf("\nSTE: ");
  for (uint8_t i = 0; i < N_FRAMES; i++)
    printf("%.3f ", feat_ste[i]);
  printf("\n");
}

/* ------------------------------------------------------------------ */
/*  Nearest-neighbour classifier                                       */
/* ------------------------------------------------------------------ */
int nearest_neighbor(float *zcr_in, float *ste_in) {
  float min_dist = 3.4e+38f;
  int best = 0;

  for (uint8_t i = 0; i < N_CLASSES; i++) {
    const float *ref_zcr = (const float *)pgm_read_word(&zcr_table[i]);
    const float *ref_ste = (const float *)pgm_read_word(&ste_table[i]);
    float dist = 0.0f;

    for (uint8_t k = 0; k < N_FRAMES; k++) {
      float dz = zcr_in[k] - pgm_read_float(&ref_zcr[k]);
      float ds = ste_in[k] - pgm_read_float(&ref_ste[k]);
      dist += dz * dz + ds * ds;
    }

    // printf("%s dist: %.4f\n", labels[i], dist);   /* handy for tuning */

    if (dist < min_dist) {
      min_dist = dist;
      best = i;
    }
  }
  return best;
}

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */
void project_Init(void) {
  UART_init(9600);
  stdout = &uart_str;
  SPI_init();
  SRAM_init();
  Timer1_init();
  ADC_init();
  init_INT0();
  LCD_Clear();
  LCD_Init();
  _delay_ms(50);
  sei();
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */
int main(void) {
  project_Init();
  _delay_ms(50);

  while (1) {
    if (button_event) {
      button_event = 0;
      _delay_ms(20);
      if (!(PIND & (1 << PD2))) {
        recording = !recording;
        PORTD ^= (1 << PD7);
      }
    }

    /* --- rising edge: start of recording --- */
    if (recording && !prev_recording) {
      _delay_ms(20);
      prev_recording = 1;
      address = 0;
      number_of_samples = 0;
    }
    /* --- falling edge: end of recording --- */
    else if (!recording && prev_recording) {
      _delay_ms(20);
      prev_recording = 0;
      printf("Processing... (%u samples)\n", number_of_samples);

      compute_features(number_of_samples, zcr_arr, ste_arr);

      int label = nearest_neighbor(zcr_arr, ste_arr);

      LCD_Clear();

      LCD_String_xy(0, 0, (char *)labels[label]);
      printf("Result: %s\n", labels[label]);
      if (label == 0) {
        PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
      } else if (label == 1) {
        PORTB |= (1 << PB0);
        PORTB &= ~(1 << PB1);
        PORTB &= ~(1 << PB2);
      } else if (label == 2) {
        PORTB &= ~(1 << PB0);
        PORTB |= (1 << PB1);
        PORTB &= ~(1 << PB2);
      } else if (label == 3) {
        PORTB |= (1 << PB0);
        PORTB |= (1 << PB1);
        PORTB &= ~(1 << PB2);
      } else if (label == 4) {
        PORTB &= ~(1 << PB0);
        PORTB &= ~(1 << PB1);
        PORTB |= (1 << PB2);
      }
      _delay_ms(100);
    }

    /* --- store incoming sample --- */
    if (sample_ready) {
      sample_ready = 0;
      if (number_of_samples < 12000) {
        SRAM_write_byte(address, audio_sample_raw);
        address++;
        number_of_samples++;
      }
    }
  }
  return EXIT_SUCCESS;
}