#define F_CPU 11059200UL
#include "speech_processing.h"
#include "speech_data.h"        
#include "ADC_interface.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h> 
#include <math.h>

#define BAUD_PRESCALE  ((F_CPU / (16UL * USAR_BAUDRATE)) - 1)
#define SPEECH_THRESHOLD 30
#define WINDOW_SIZE 64
// ==================== GLOBALS ====================
// All buffers sized for FFT_SIZE=64 ? total SRAM: ~400 bytes
volatile int8_t g_adc_value = 0;
volatile uint8_t g_sampling_flag = 0;

//static int16_t fft_re[FFT_SIZE]; // 128 bytes ? reused for input + FFT real
//static int16_t fft_im[FFT_SIZE]; // 128 bytes ? FFT imaginary
//static uint16_t mag_sq[FFT_SIZE / 2]; //  64 bytes ? magnitude squared per bin
//static int32_t mel_e[N_MELS]; //  52 bytes ? mel filter energies
//static uint8_t log_mel[N_MELS]; //  13 bytes ? log2 approximation
//static int8_t mfcc_buf[N_MFCC]; //  13 bytes ? final output

static float feature_out[4];
static int8_t audio_samples[WINDOW_SIZE];

// ==================== TIMER + ADC ISR ====================

void SP_Init(void) {
    // CTC on Timer0, prescaler 64 ? OCR0 = F_CPU/(64*8000) - 1 = 21
    // Gives ~8kHz interrupt rate; ADC prescaler 128 gives 86kHz ADC clock
    // Actual sample rate set by timer: 8000 Hz
    // (re-extract Python features at sr=8000 to match)
    TCCR0 = (1 << WGM01) | (1 << CS01) | (1 << CS00);
    OCR0 = 21;
    TIMSK |= (1 << OCIE0);
}

ISR(TIMER0_COMP_vect) {
    // Minimal ISR ? just trigger and read
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADIF)); // ~9us at 86kHz ADC clock, safe in ISR
    g_adc_value = ADC;
    g_sampling_flag = 1;
}

//// ==================== HAMMING WINDOW ====================
//
//static void apply_hamming(void) {
//    for (uint8_t i = 0; i < FFT_SIZE; i++) {
//        int16_t w = pgm_read_word(&hamming_q15[i]);
//        fft_re[i] = (int16_t) (((int32_t) fft_re[i] * w) >> 15);
//    }
//}
//
//// ==================== 64-POINT FIXED-POINT FFT ====================
//
//static void fft64(void) {
//    // Bit-reversal permutation
//    for (uint8_t i = 0; i < FFT_SIZE; i++) {
//        uint8_t j = pgm_read_byte(&bit_rev_64[i]);
//        if (j > i) {
//            int16_t t = fft_re[i];
//            fft_re[i] = fft_re[j];
//            fft_re[j] = t;
//            // fft_im is zero-initialized for real input, no swap needed
//        }
//    }
//
//    // Butterfly stages (log2(64) = 6 stages)
//    uint8_t half = 1;
//    for (uint8_t stage = 0; stage < 6; stage++) {
//        uint8_t step = half << 1;
//        uint8_t twiddle_step = 32 >> stage;
//
//        for (uint8_t k = 0; k < half; k++) {
//            int16_t wr = pgm_read_word(&twiddle_cos[k * twiddle_step]);
//            int16_t wi = pgm_read_word(&twiddle_sin[k * twiddle_step]);
//
//            for (uint8_t j = k; j < FFT_SIZE; j += step) {
//                uint8_t m = j + half;
//                int16_t tr = (int16_t) (((int32_t) wr * fft_re[m]
//                        - (int32_t) wi * fft_im[m]) >> 15);
//                int16_t ti = (int16_t) (((int32_t) wr * fft_im[m]
//                        + (int32_t) wi * fft_re[m]) >> 15);
//                fft_re[m] = fft_re[j] - tr;
//                fft_im[m] = fft_im[j] - ti;
//                fft_re[j] += tr;
//                fft_im[j] += ti;
//            }
//        }
//        half = step;
//    }
//}
//
//// ==================== MAGNITUDE SQUARED ====================
//
//static void compute_mag_sq(void) {
//    for (uint8_t i = 0; i < FFT_SIZE / 2; i++) {
//        // Shift right to keep within uint16 range
//        mag_sq[i] = (uint16_t) (
//                ((int32_t) fft_re[i] * fft_re[i]
//                + (int32_t) fft_im[i] * fft_im[i]) >> 10
//                );
//    }
//}
//
//// ==================== MEL FILTERBANK ====================
//
//static void apply_mel(void) {
//    for (uint8_t m = 0; m < N_MELS; m++) {
//        uint8_t f0 = pgm_read_byte(&mel_bins[m][0]);
//        uint8_t fp = pgm_read_byte(&mel_bins[m][1]);
//        uint8_t f1 = pgm_read_byte(&mel_bins[m][2]);
//        int32_t e = 0;
//
//        // Rising slope
//        if (fp > f0) {
//            for (uint8_t f = f0; f < fp; f++) {
//                uint8_t w = (uint8_t) (255UL * (f - f0) / (fp - f0));
//                e += ((uint32_t) mag_sq[f] * w) >> 8;
//            }
//        }
//        // Falling slope
//        if (f1 > fp) {
//            for (uint8_t f = fp; f < f1; f++) {
//                uint8_t w = (uint8_t) (255UL * (f1 - f) / (f1 - fp));
//                e += ((uint32_t) mag_sq[f] * w) >> 8;
//            }
//        }
//        mel_e[m] = e;
//    }
//}
//
//// ==================== INTEGER LOG2 ====================
//
//static uint8_t log2_u32(uint32_t x) {
//    if (x == 0) return 0;
//    uint8_t n = 0;
//    while (x >>= 1) n++;
//    return n;
//}
//
//// ==================== DCT ====================
//
//static void apply_dct(int8_t* out) {
//    for (uint8_t k = 0; k < N_MFCC; k++) {
//        int16_t sum = 0;
//        for (uint8_t n = 0; n < N_MELS; n++) {
//            int8_t d = pgm_read_byte(&dct_matrix[k][n]);
//            sum += (int16_t) d * (int16_t) log_mel[n];
//        }
//        sum >>= 7; // undo Q7 scaling
//        if (sum > 127) sum = 127;
//        if (sum < -128) sum = -128;
//        out[k] = (int8_t) sum;
//    }
//}

void UART_init(long USAR_BAUDRATE) {
    UBRRL = BAUD_PRESCALE;

    UBRRH = (BAUD_PRESCALE >> 8);

    UCSRB |= (1 << RXEN) | (1 << TXEN);

    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
}

uint8_t UART_dataAvailable(void) {
    return (UCSRA & (1 << RXC));
}

int UART_getChar(FILE * stream) {
    while ((UCSRA & (1 << RXC)) == 0);

    return UDR;
}

int UART_putChar(char c, FILE * stream) {
    while (!(UCSRA & (1 << UDRE)));

    UDR = c;

    return 0;
}

static FILE uart_str = FDEV_SETUP_STREAM(UART_putChar, UART_getChar, _FDEV_SETUP_RW);

static float compute_zcr(int8_t* samples, uint16_t size) {
    float zero_crossings = 0;

    for (uint8_t i = 1; i < size; i++) {
        if ((samples[i - 1] < 0 && samples[i] >= 0) || (samples[i - 1] >= 0 && samples[i] < 0)) {
            zero_crossings++;
        }
    }

    float zero_crossings_scaled = (zero_crossings / WINDOW_SIZE);

//    printf("DEBUG ZCR: crossings=%d, scaled=%d\n", zero_crossings, zero_crossings_scaled);
    return zero_crossings_scaled;
}

float compute_ste(int8_t *frame, uint16_t len) {
    float sum = 0.0f;
    for (uint8_t i = 0; i < len; i++) {
        float s = (float) frame[i] / 256.0f; // normalize to [-1.0, 1.0]
        sum += pow(s, 2);
    }
    return sum / len; // normalized STE (matches rms)
}




// ==================== MAIN FEATURE EXTRACTION ====================

void SP_UART_Init(void) {
    UART_init(9600);
    stdin = stdout = &uart_str;
    _delay_ms(50);
}

void SP_Compute_features(float *out) {
    uint8_t num_windows = 0;
    float   zcr_acc = 0.0f;
    float   ste_acc = 0.0f;

    while (num_windows < 8) {

        // 1. Collect raw uint8_t samples
        for (uint8_t i = 0; i < WINDOW_SIZE; i++) {
            while (!g_sampling_flag);
            g_sampling_flag = 0;
            audio_samples[i] = g_adc_value;
            if(num_windows == 0){
                printf("ADC value: %d\n",g_adc_value);
            }
        }

        // 2. Compute integer mean
        uint16_t sum = 0;
        for (uint8_t i = 0; i < WINDOW_SIZE; i++) sum += audio_samples[i];
        uint8_t mean_int = (uint8_t)(sum / WINDOW_SIZE);

        // 3. Center into int8_t buffer
        for (uint8_t i = 0; i < WINDOW_SIZE; i++) {
            audio_samples[i] = (int8_t)((int16_t)audio_samples[i] - mean_int);
        }

        zcr_acc += compute_zcr(audio_samples, WINDOW_SIZE);
        ste_acc += compute_ste(audio_samples, WINDOW_SIZE);
        num_windows++;
    }

    out[0] = zcr_acc / num_windows;
    out[1] = ste_acc / num_windows;

    printf_P(PSTR("ZCR=%f, STE=%f\n"), out[0], out[1]);
}

// ==================== FULL MFCC PIPELINE ====================

//void SP_Compute_MFCC(int8_t* out) {
//
//    UART_init(9600);
//    stdin = stdout = &uart_str;
//    _delay_ms(50);
//    
//    // 1. Collect FFT_SIZE samples into fft_re[]
//    memset(fft_im, 0, sizeof (fft_im));
//    for (uint8_t i = 0; i < FFT_SIZE; i++) {
//        while (!g_sampling_flag);
//        g_sampling_flag = 0;
//        fft_re[i] = (int16_t) g_adc_value - 512;
//    }
//
//    // 2. Hamming window
//    apply_hamming();
//
//    // 3. FFT
//    fft64();
//
//    // 4. Magnitude squared (first 32 bins only ? real FFT symmetry)
//    compute_mag_sq();
//
//    // 5. Mel filterbank
//    apply_mel();
//
//    // 6. Log2 of each mel energy
//    for (uint8_t m = 0; m < N_MELS; m++) {
//        log_mel[m] = log2_u32((uint32_t) mel_e[m]);
//    }
//
//    // 7. DCT ? MFCC coefficients
//    apply_dct(out);
//
//    for (int i = 0; i < N_MFCC; i++) {
//        printf("%d,", out[i]); // or "%d " for same line
//    }
//    printf("\n");
//
//}
//
//// ==================== SPEECH DETECTION ====================
//
//uint8_t SP_Is_Speaking(void) {
//    uint16_t sample = ADC_u16ReadChannel(0);
//    int16_t centered = (int16_t) sample - 512;
//    return (abs(centered) > THRESHOLD);
//}