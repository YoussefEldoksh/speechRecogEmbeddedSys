#define F_CPU 11059200UL

#include "ADC_interface.h"

void ADC_init() {
    ADMUX = (1 << REFS0) | (1 << ADLAR); // AVCC, left adjust

    // prescaler 64
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

int8_t ADC_read() {
    /*Select the Channel (Masking first 5 bits of ADMUX to clear old channel) */
    /* Channel must be between 0-7 */
//    ADMUX &= 0xE0;
//    ADMUX |= (channel & 0x07);

    /*Start Conversion */
    ADCSRA |= (1 << ADSC);

    /*Wait for conversion to complete (ADSC becomes 0 when finished) */
    while (ADCSRA & (1 << ADSC));
    return ADCH;
}

//uint8_t ADC_u8ReadChannel(uint8_t channel) {
//    /*a wrapper to return the top 8 bits to save memory */
//    return (uint8_t)(ADC_u16ReadChannel(channel) >> 2);
//}