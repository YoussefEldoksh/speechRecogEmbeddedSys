#ifndef ADC_INTERFACE_H_
#define ADC_INTERFACE_H_

#include <avr/io.h>


void ADC_init(void);
uint16_t ADC_u16ReadChannel(uint8_t channel);
uint8_t ADC_u8ReadChannel(uint8_t channel);

#endif /* ADC_INTERFACE_H_ */