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
#include <stdio.h>
#pragma once

#define THRESHOLD   30

void SP_Init(void);
void SP_Compute_features(float* out);
uint8_t  SP_Is_Speaking(void);
void UART_init(long USAR_BAUDRATE);
uint8_t UART_dataAvailable(void);
int UART_getChar(FILE * stream);
int UART_putChar(char c, FILE * stream);
void SP_UART_Init(void) ;


#ifdef	__cplusplus 
}
#endif

#endif	/* SPEECH_PROCESSING_H */

