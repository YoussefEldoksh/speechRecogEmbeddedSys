#include "ADC_interface.h"

void ADC_init(void) {
    /* Select Voltage Reference: AVCC with external capacitor at AREF pin */
    /* REFS1=0, REFS0=1 */
    ADMUX = (1 << REFS0) | (1 << ADLAR); // left adjust for 8-bit reads (ADCH)

    /*Set ADC Prescaler to 128 */
    /* 11,059,200 / 128 = 86.4 kHz (Inside the 50-200kHz range for 10-bit accuracy) */
    /* ADPS2=1, ADPS1=1, ADPS0=1 */
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /*Enable the ADC Peripheral */
    ADCSRA |= (1 << ADEN);
}

uint16_t ADC_u16ReadChannel(uint8_t channel) {
    /*Select the Channel (Masking first 5 bits of ADMUX to clear old channel) */
    /* Channel must be between 0-7 */
    ADMUX &= 0xE0;
    ADMUX |= (channel & 0x07);

    /*Start Conversion */
    ADCSRA |= (1 << ADSC);

    /*Wait for conversion to complete (ADSC becomes 0 when finished) */
    while (ADCSRA & (1 << ADSC));
    return ADCH;
}

uint8_t ADC_u8ReadChannel(uint8_t channel) {
    /* Return the top 8 bits (left-adjusted) */
    return (uint8_t)ADC_u16ReadChannel(channel);
}