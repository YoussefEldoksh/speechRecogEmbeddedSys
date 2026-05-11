#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_FEATURES 8

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_down[N_FEATURES] PROGMEM = {4, 139, 81, 95, 108, 119, 119, 119};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_left[N_FEATURES] PROGMEM = {6, 139, 83, 97, 108, 119, 119, 119};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_off[N_FEATURES] PROGMEM = {4, 139, 81, 95, 107, 118, 118, 118};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_on[N_FEATURES] PROGMEM = {9, 139, 81, 96, 110, 122, 122, 122};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_right[N_FEATURES] PROGMEM = {9, 139, 81, 96, 109, 121, 121, 121};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_start[N_FEATURES] PROGMEM = {5, 139, 82, 96, 110, 121, 121, 121};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_stop[N_FEATURES] PROGMEM = {5, 139, 80, 94, 106, 116, 116, 116};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_up[N_FEATURES] PROGMEM = {4, 138, 78, 92, 103, 114, 114, 114};

// Pointer table � index matches LABELS[] below
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

// Word labels
static const char* const LABELS[] PROGMEM = {"down", "left", "off", "on", "right", "start", "stop", "up"};

#define N_WORDS 8

#endif // WORD_FEATURES_H
