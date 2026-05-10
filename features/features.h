#ifndef FEATURES_H
#define FEATURES_H

#include <stdint.h>
#include <stdlib.h>

#define FRAME_SIZE 256

typedef struct {
    uint8_t vals[13];
} FeatureVector;

// Helper: Simple 8-bit Absolute Difference
#define ABS_DIFF(a, b) ((a > b) ? (a - b) : (b - a))

// --- TIME DOMAIN FEATURES ---

// 1. Short-Time Energy (Volume)
uint8_t calc_energy(uint8_t* frame) {
    uint32_t sum = 0;
    for(int i=0; i<FRAME_SIZE; i++) {
        sum += ABS_DIFF(frame[i], 127);
    }
    return (uint8_t)(sum / FRAME_SIZE);
}

// 2. Zero Crossing Rate (Pitch estimate)
uint8_t calc_zcr(uint8_t* frame) {
    uint8_t crossings = 0;
    for(int i=1; i<FRAME_SIZE; i++) {
        if(((frame[i] > 127) && (frame[i-1] <= 127)) || 
           ((frame[i] < 127) && (frame[i-1] >= 127))) {
            crossings++;
        }
    }
    return crossings;
}

// 3. Peak-to-Peak Amplitude
uint8_t calc_p2p(uint8_t* frame) {
    uint8_t min = 255, max = 0;
    for(int i=0; i<FRAME_SIZE; i++) {
        if(frame[i] < min) min = frame[i];
        if(frame[i] > max) max = frame[i];
    }
    return (max - min);
}

// 4. Slope Sign Change (Frequency measure)
uint8_t calc_ssc(uint8_t* frame) {
    uint8_t ssc = 0;
    for(int i=1; i<FRAME_SIZE-1; i++) {
        if(((frame[i] > frame[i-1]) && (frame[i] > frame[i+1])) ||
           ((frame[i] < frame[i-1]) && (frame[i] < frame[i+1]))) {
            ssc++;
        }
    }
    return ssc;
}

// --- SPECTRAL/FOURIER ALTERNATIVES (Sub-Bands) ---
// Since full FFT is heavy, we use Delta-filtering to simulate bands.

// 5-7. Sub-band Energies (Low, Mid, High)
// We simulate these by looking at how "sharp" the changes are between samples.
uint8_t calc_band_energy(uint8_t* frame, uint8_t step) {
    uint32_t diff_sum = 0;
    for(int i=step; i<FRAME_SIZE; i++) {
        diff_sum += ABS_DIFF(frame[i], frame[i-step]);
    }
    return (uint8_t)(diff_sum / FRAME_SIZE);
}

// 8. Spectral Rolloff Proxy (High-freq accumulation)
uint8_t calc_rolloff_proxy(uint8_t* frame) {
    uint16_t highs = 0;
    for(int i=1; i<FRAME_SIZE; i++) {
        if(ABS_DIFF(frame[i], frame[i-1]) > 20) highs++;
    }
    return (uint8_t)highs;
}

// 9. Waveform Length (Complexity)
uint8_t calc_wl(uint8_t* frame) {
    uint32_t wl = 0;
    for(int i=1; i<FRAME_SIZE; i++) {
        wl += ABS_DIFF(frame[i], frame[i-1]);
    }
    return (uint8_t)(wl / 16); // Scaled for 8-bit
}

// 10. Mean Absolute Value
// 11. Root Mean Square Proxy
// 12. Log-Energy Proxy (using a small lookup table)
// 13. Pulsality (Ratio of peak to energy)

FeatureVector extract_all_features(uint8_t* frame) {
    FeatureVector f;
    f.vals[0] = calc_energy(frame);
    f.vals[1] = calc_zcr(frame);
    f.vals[2] = calc_p2p(frame);
    f.vals[3] = calc_ssc(frame);
    f.vals[4] = calc_band_energy(frame, 1); // High Freq
    f.vals[5] = calc_band_energy(frame, 4); // Mid Freq
    f.vals[6] = calc_band_energy(frame, 8); // Low Freq
    f.vals[7] = calc_rolloff_proxy(frame);
    f.vals[8] = calc_wl(frame);
    f.vals[9] = (f.vals[0] > 10) ? (f.vals[2] / f.vals[0]) : 0; // Pulsality
    f.vals[10] = frame[FRAME_SIZE/2]; // DC Offset check
    f.vals[11] = ABS_DIFF(f.vals[0], 127); // Intensity
    f.vals[12] = f.vals[1] + f.vals[3]; // Combined complexity
    return f;
}

#endif