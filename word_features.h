#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_FEATURES 8

// ZCR, log-STE, Mel[0..5]
static const float feat_down[N_FEATURES] PROGMEM = {0.300377f, -4.359515f, -6.064520f, -6.188857f, -6.566078f, -6.568796f, -6.896235f, -7.148645f};

// ZCR, log-STE, Mel[0..5]
static const float feat_left[N_FEATURES] PROGMEM = {0.363822f, -4.537633f, -6.674198f, -6.992841f, -7.331271f, -6.911180f, -6.745756f, -6.852909f};

// ZCR, log-STE, Mel[0..5]
static const float feat_off[N_FEATURES] PROGMEM = {0.369501f, -4.734900f, -6.737294f, -6.770664f, -6.777926f, -6.810320f, -7.119432f, -7.441887f};

// ZCR, log-STE, Mel[0..5]
static const float feat_on[N_FEATURES] PROGMEM = {0.302526f, -3.925523f, -5.832629f, -6.071343f, -6.204804f, -6.296070f, -6.602297f, -6.643975f};

// ZCR, log-STE, Mel[0..5]
static const float feat_right[N_FEATURES] PROGMEM = {0.321640f, -3.972558f, -6.367244f, -6.486142f, -6.976037f, -6.647659f, -6.658583f, -6.957759f};

// ZCR, log-STE, Mel[0..5]
static const float feat_start[N_FEATURES] PROGMEM = {0.376833f, -4.188819f, -6.270663f, -5.959852f, -6.431057f, -6.237648f, -6.361084f, -6.468153f};

// ZCR, log-STE, Mel[0..5]
static const float feat_stop[N_FEATURES] PROGMEM = {0.328468f, -4.478813f, -7.116247f, -7.158666f, -7.364592f, -7.420154f, -7.444934f, -7.357604f};

// ZCR, log-STE, Mel[0..5]
static const float feat_up[N_FEATURES] PROGMEM = {0.292927f, -4.614080f, -7.058267f, -6.947430f, -7.381168f, -7.570995f, -7.761914f, -7.732168f};

// Pointer table — index matches LABELS[] below
static const float* const feat_table[] PROGMEM = {
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
