#ifndef SPEECH_DATA_H
#define SPEECH_DATA_H

#include <avr/pgmspace.h>

#define N_FEATURES 10
#define PRE_EMPHASIS 0.97f

static const uint16_t feat_down[10] PROGMEM = {141, 138, 170, 182, 137, 121, 123, 111, 133, 7106};
static const uint16_t feat_left[10] PROGMEM = {117, 114, 118, 113, 137, 167, 156, 97, 121, 7712};
static const uint16_t feat_off[10] PROGMEM = {119, 108, 149, 135, 114, 120, 131, 85, 113, 7401};
static const uint16_t feat_on[10] PROGMEM = {143, 168, 205, 206, 143, 142, 162, 164, 164, 7357};
static const uint16_t feat_right[10] PROGMEM = {118, 154, 160, 158, 188, 176, 179, 145, 157, 7603};
static const uint16_t feat_start[10] PROGMEM = {120, 136, 168, 175, 153, 158, 176, 145, 152, 7793};
static const uint16_t feat_stop[10] PROGMEM = {111, 90, 120, 129, 106, 104, 131, 119, 112, 7836};
static const uint16_t feat_up[10] PROGMEM = {114, 85, 144, 127, 88, 93, 101, 96, 102, 7443};

// Pointer table — order must match labels[] in main.c
static const uint16_t* const feat_table[] PROGMEM = {
    feat_down,
    feat_left,
    feat_off,
    feat_on,
    feat_right,
    feat_start,
    feat_stop,
    feat_up,
};

#endif
