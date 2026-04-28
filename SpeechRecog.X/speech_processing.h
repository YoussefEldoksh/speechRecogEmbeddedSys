/* 
 * File:   speech_processing.h
 * Author: Lenovo
 *
 * Created on April 22, 2026, 9:55 PM
 */

#ifndef SPEECH_PROCESSING_H
#define	SPEECH_PROCESSING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#pragma once

#define N_MELS      13
#define THRESHOLD   20

/*
 * Hardware notes (ATmega32A + MAX microphone module):
 * - This project targets ATmega32A running at 11.0592 MHz.
 * - Microphone analog output should be connected to ADC0 (PA0).
 * - If your MAX-based module exposes a digital VAD/DOUT, you may connect it
 *   to INT0 (PD2) to trigger the external interrupt for faster wake-up.
 * - ADC reference: AVCC (configured in `adc/adc.c`). Ensure AREF capacitor is fitted.
 * - Consider converting float features to fixed-point (Q15 or Q8) for faster, smaller
 *   code on AVR; I can convert automatically if you prefer.
 */

void SP_Init(void);
void SP_Compute_features(int16_t* out);
uint8_t  SP_Is_Speaking(void);
void UART_init(long USART_BAUDRATE);
uint8_t UART_dataAvailable(void);
int UART_getChar(FILE * stream);
int UART_putChar(char c, FILE * stream);
void SP_UART_Init(void) ;


#ifdef	__cplusplus 
}
#endif

#endif	/* SPEECH_PROCESSING_H */

