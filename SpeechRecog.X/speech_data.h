#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_CLASSES 5
#define N_FRAMES  8

// ---- ZCR reference arrays (one per word) ----
static const float back_zcr[N_FRAMES] PROGMEM = {0.090, 0.137, 0.369, 0.615, 0.580, 0.474, 0.329, 0.052};
static const float go_zcr[N_FRAMES] PROGMEM = {0.133, 0.156, 0.497, 0.810, 0.769, 0.521, 0.329, 0.076};
static const float one_zcr[N_FRAMES] PROGMEM = {0.144, 0.283, 0.522, 0.737, 0.539, 0.259, 0.180, 0.070};
static const float ship_zcr[N_FRAMES] PROGMEM = {0.047, 0.198, 0.659, 0.540, 0.286, 0.084, 0.063, 0.024};
static const float acht_zcr[N_FRAMES] PROGMEM = {0.376, 0.369, 0.598, 0.831, 0.621, 0.202, 0.236, 0.129};


// ---- STE reference arrays (one per word) ----
static const float back_ste[N_FRAMES] PROGMEM = {0.015, 0.079, 0.407, 0.701, 0.543, 0.261, 0.047, 0.004};
static const float go_ste[N_FRAMES] PROGMEM = {0.004, 0.059, 0.359, 0.744, 0.601, 0.293, 0.097, 0.022};
static const float one_ste[N_FRAMES] PROGMEM = {0.008, 0.049, 0.355, 0.679, 0.438, 0.146, 0.013, 0.012};
static const float ship_ste[N_FRAMES] PROGMEM = {0.027, 0.033, 0.311, 0.673, 0.319, 0.281, 0.127, 0.047};
static const float acht_ste[N_FRAMES] PROGMEM = {0.089, 0.228, 0.763, 0.779, 0.243, 0.085, 0.068, 0.021};

// ---- Pointer tables indexed by class ----
static const float * const zcr_table[N_CLASSES] PROGMEM = {back_zcr, go_zcr, one_zcr, ship_zcr, acht_zcr };
static const float * const ste_table[N_CLASSES] PROGMEM = {back_ste, go_ste, one_ste, ship_ste, acht_ste};

// ---- Labels ----
static const char * const LABELS[N_CLASSES] PROGMEM = {"back", "go", "one", "ship", "acht"};

#endif // WORD_FEATURES_H