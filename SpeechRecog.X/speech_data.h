#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_FEATURES 8

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_down[N_FEATURES] PROGMEM = {21, 101, 92, 102, 103, 108, 101, 95};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_left[N_FEATURES] PROGMEM = {31, 91, 77, 87, 93, 104, 103, 101};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_off[N_FEATURES] PROGMEM = {33, 86, 73, 77, 90, 103, 102, 98};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_on[N_FEATURES] PROGMEM = {19, 97, 88, 100, 95, 102, 96, 92};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_right[N_FEATURES] PROGMEM = {26, 91, 78, 94, 103, 108, 100, 94};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_start[N_FEATURES] PROGMEM = {33, 88, 75, 89, 97, 106, 100, 98};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_stop[N_FEATURES] PROGMEM = {29, 85, 71, 82, 90, 95, 92, 93};

// ZCR, log-STE, Mel[0..5]
static const uint16_t feat_up[N_FEATURES] PROGMEM = {20, 84, 71, 78, 90, 92, 86, 86};

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
