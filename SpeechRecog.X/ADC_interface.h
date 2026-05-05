/* 
 * File:   ADC_interface.h
 * Author: Lenovo
 *
 * Created on April 22, 2026, 9:54 PM
 */

#ifndef ADC_INTERFACE_H
#define	ADC_INTERFACE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <avr/io.h>


void ADC_init(void);
int8_t ADC_read(void);
uint8_t ADC_u8ReadChannel(uint8_t channel);



#ifdef	__cplusplus
}
#endif

#endif	/* ADC_INTERFACE_H */

