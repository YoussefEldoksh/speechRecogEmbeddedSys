#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_FEATURES 8

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_down[N_FEATURES] PROGMEM = {20, 79, 58, 77, 77, 93, 90, 89};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_left[N_FEATURES] PROGMEM = {21, 78, 41, 67, 71, 87, 88, 87};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_off[N_FEATURES] PROGMEM = {12, 77, 31, 56, 69, 86, 88, 88};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_on[N_FEATURES] PROGMEM = {20, 78, 55, 75, 74, 91, 90, 90};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_right[N_FEATURES] PROGMEM = {23, 79, 48, 73, 79, 93, 96, 90};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_start[N_FEATURES] PROGMEM = {34, 80, 50, 76, 82, 96, 97, 96};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_stop[N_FEATURES] PROGMEM = {26, 78, 40, 63, 72, 86, 90, 93};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_up[N_FEATURES] PROGMEM = {10, 76, 41, 58, 66, 79, 84, 87};

// Pointer table - index matches LABELS[] below
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
static const char label_down[] PROGMEM = "down";
static const char label_left[] PROGMEM = "left";
static const char label_off[] PROGMEM = "off";
static const char label_on[] PROGMEM = "on";
static const char label_right[] PROGMEM = "right";
static const char label_start[] PROGMEM = "start";
static const char label_stop[] PROGMEM = "stop";
static const char label_up[] PROGMEM = "up";

static const char* const LABELS[] PROGMEM = {
    label_down,
    label_left,
    label_off,
    label_on,
    label_right,
    label_start,
    label_stop,
    label_up,
};

#define N_WORDS 8

#endif // WORD_FEATURES_H
