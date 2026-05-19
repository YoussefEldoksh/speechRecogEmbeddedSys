#define F_CPU 11059200UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

volatile uint8_t recording = 0;
volatile uint8_t prev_recording = 0;
volatile uint8_t audio_sample = 0;
volatile uint8_t sample_ready = 0;
volatile int number_of_samples = 0;
volatile uint16_t address = 0;

// ================= UART =================

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

void UART_init(long baudrate)
{
    uint16_t prescale = (F_CPU / (16UL * baudrate)) - 1;

    UBRRL = prescale & 0xFF;
    UBRRH = prescale >> 8;

    UCSRB = (1 << TXEN); // TX only
    UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
}

void send_byte(uint8_t data)
{
    while (!(UCSRA & (1 << UDRE)))
        ;
    UDR = data;
}

void send_string(const char *str)
{
    while (*str)
    {
        send_byte(*str++);
    }
}

// ================= ADC =================

void ADC_init()
{
    ADMUX = (1 << REFS0) | (1 << ADLAR); // AVCC, left adjust

    // prescaler 64
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint8_t ADC_read()
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADCH;
}

// ================= TIMER1 (8kHz) =================

void Timer1_init()
{
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS11);  // prescaler 8

    OCR1A = (F_CPU / (8UL * 8000UL)) - 1; // 8kHz

    TIMSK |= (1 << OCIE1A);
}

// ================= INT0 (Button) =================

void INT0_init()
{
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2); // pull-up

    MCUCR |= (1 << ISC01); // falling edge
    GICR |= (1 << INT0);
}

// ================= ISR =================

ISR(INT0_vect)
{
    recording = !recording;
    PORTB ^= (1 << PB0);
}

// Timer interrupt: sample audio

ISR(TIMER1_COMPA_vect)
{
    if (!recording)
        return;

    audio_sample = ADC_read();
    sample_ready = 1;
}

// ================= MAIN =================

int main(void)
{
    UART_init(115200);
    ADC_init();
    Timer1_init();
    INT0_init();
    SPI_init();
    SRAM_init();

    DDRB |= (1 << PB0); // LED
    PORTB |= (1 << PB0);
    DDRD |= (1 << PD7);
    PORTD &= ~(1 << PD7);
    sei();

    while (1)
    {
        // START / STOP messages
        if (recording && !prev_recording)
        {
            send_string("START\n");
            prev_recording = 1;
            _delay_ms(50);
        }
        else if (!recording && prev_recording)
        {

            prev_recording = 0;

            for (int i = 0; i < number_of_samples; i++)
            {
                int8_t val = SRAM_read_byte(i);
                send_byte(val);
            } 

            _delay_ms(50);
        }

        // send audio outside ISR (safe)
        if (sample_ready)
        {
            if (number_of_samples < 8000)
            {
                SRAM_write_byte(address, audio_sample);
                address++;
                number_of_samples++;
            }
            sample_ready = 0;
        }
    }
    return 0;
}