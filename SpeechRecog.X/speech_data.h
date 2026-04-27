#ifndef SPEECH_DATA_H
#define SPEECH_DATA_H

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>

// ==================== DIMENSIONS ====================
#define N_MFCC    2
//#define N_MELS    4
#define N_CLASSES  8
#define FFT_SIZE  512
#define THRESHOLD 30
#define SAMPLE_RATE 8000    

// ==================== WORD TEMPLATES ====================
//static const int8_t PROGMEM mfcc_on[]    = {-128, 99, 5, -2, 9, -9, -5, -3, 0, 5, -16, 1, -18};
//static const int8_t PROGMEM mfcc_off[]   = {-128, 89, -3, -27, 3, -10, 4, -8, 6, 4, -13, -1, -19};
//static const int8_t PROGMEM mfcc_up[]    = {-128, 98, -11, -21, 5, -7, -2, 1, 5, 2, -17, -5, -13};
//static const int8_t PROGMEM mfcc_down[]  = {-128, 74, -8, 1, -12, -7, -2, 4, -11, 3, -16, -5, -14};
//static const int8_t PROGMEM mfcc_left[]  = {-128, 39, 19, 2, -14, -11, -2, -3, -12, 11, -16, -4, -9};
//static const int8_t PROGMEM mfcc_right[] = {-128, 67, -10, -3, -27, -13, -17, 9, -7, -6, -11, -4, -11};
//static const int8_t PROGMEM mfcc_start[] = {-128, 87, -8, -5, -4, -15, -16, 2, -1, 3, -8, -2, -17};
//static const int8_t PROGMEM mfcc_stop[]  = {-128, 80, -8, -20, 2, -15, -8, -4, -5, 8, -16, -3, -22};


 static const float PROGMEM features_on[] = { 0.1350, 0.0167 };
 static const float PROGMEM features_off[] = { 0.2156, 0.0181 };
 static const float PROGMEM features_up[] = { 0.1534, 0.0141 };
 static const float PROGMEM features_down[] = { 0.1098, 0.0205 };
 static const float PROGMEM features_left[] = { 0.2541, 0.0153 };
 static const float PROGMEM features_right[] = { 0.1625, 0.0186 };
 static const float PROGMEM features_start[] = { 0.2350, 0.0129 };
 static const float PROGMEM features_stop[] = { 0.2326, 0.0125 };


// static const int8_t* const PROGMEM mfcc_table[N_CLASSES] = {
//     mfcc_on, mfcc_off, mfcc_up, mfcc_down,
//     mfcc_right, mfcc_left, mfcc_start, mfcc_stop
// };

static const float* const PROGMEM features_table[N_CLASSES] = {
    features_on, features_off, features_up, features_down,
    features_right, features_left, features_start, features_stop
};
//
//// extern declaration ? definition goes in one .c file only
//extern const char* labels[N_CLASSES];
//
//// ==================== HAMMING WINDOW Q15 ====================
//static const int16_t PROGMEM hamming_q15[64] = {
//     1072,  1154,  1399,  1806,  2372,  3090,  3954,  4956,
//     6087,  7336,  8692, 10143, 11674, 13271, 14921, 16609,
//    18320, 20038, 21749, 23437, 25087, 26684, 27215, 29469,
//    30762, 31974, 33094, 34110, 35013, 35794, 36446, 36962,
//    37337, 37568, 37652, 37588, 37376, 37017, 36514, 35870,
//    35089, 34178, 33143, 31991, 30730, 29366, 27908, 26364,
//    24743, 23054, 21307, 19511, 17676, 15811, 13926, 12030,
//    10133,  8244,  6374,  4533,  2730,   976,   279,     0
//};
//
//// ==================== FFT TWIDDLE FACTORS Q15 ====================
//static const int16_t PROGMEM twiddle_cos[32] = {
//    32767, 32609, 32137, 31356, 30273, 28897, 27244, 25329,
//    23169, 20787, 18204, 15446, 12539,  9511,  6392,  3211,
//        0, -3211, -6392, -9511,-12539,-15446,-18204,-20787,
//   -23169,-25329,-27244,-28897,-30273,-31356,-32137,-32609
//};
//
// static const int16_t PROGMEM twiddle_sin[32] = {
//        0, -3211, -6392, -9511,-12539,-15446,-18204,-20787,
//   -23169,-25329,-27244,-28897,-30273,-31356,-32137,-32609,
//   -32767,-32609,-32137,-31356,-30273,-28897,-27244,-25329,
//   -23169,-20787,-18204,-15446,-12539, -9511, -6392, -3211
//};
//
//// ==================== BIT REVERSAL FOR N=64 ====================
// static const uint8_t PROGMEM bit_rev_64[64] = {
//     0,32,16,48, 8,40,24,56, 4,36,20,52,12,44,28,60,
//     2,34,18,50,10,42,26,58, 6,38,22,54,14,46,30,62,
//     1,33,17,49, 9,41,25,57, 5,37,21,53,13,45,29,61,
//     3,35,19,51,11,43,27,59, 7,39,23,55,15,47,31,63
//};
//
//// ==================== MEL FILTERBANK BINS ====================
//// {start_bin, peak_bin, end_bin} for FS=8000, N_FFT=64, 13 filters
//static const uint8_t PROGMEM mel_bins[13][3] = {
//    { 1,  2,  3}, { 2,  3,  4}, { 3,  4,  5},
//    { 4,  5,  6}, { 5,  6,  8}, { 6,  8, 10},
//    { 8, 10, 12}, {10, 12, 15}, {12, 15, 18},
//    {15, 18, 22}, {18, 22, 26}, {22, 26, 31},
//    {26, 31, 32}
//};
//
//// ==================== DCT MATRIX Q7 ====================
//static const int8_t PROGMEM dct_matrix[13][13] = {
//    { 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36},
//    {127,117, 90, 49,  0,-49,-90,-117,-127,-117,-90,-49,  0},
//    {117, 49,-49,-117,-117,-49, 49, 117, 117, 49,-49,-117,-117},
//    { 90,-49,-127,-49, 90, 90,-49,-127,-49, 90, 90,-49,-127},
//    { 49,-117,-49, 117, 49,-117,-49, 117, 49,-117,-49, 117, 49},
//    {  0,-127,  0, 127,  0,-127,  0, 127,  0,-127,  0, 127,  0},
//    {-49,-117, 49, 117,-49,-117, 49, 117,-49,-117, 49, 117,-49},
//    {-90, -49,127, -49,-90,  90, 49,-127, 49,  90,-90, -49,127},
//    {-117, 49,117, -49,-117, 49,117, -49,-117, 49,117, -49,-117},
//    {-127,117,-90,  49,  0, -49, 90,-117,127,-117, 90, -49,  0},
//    {-117,117,-49, -49,117,-117, 49,  49,-117,117,-49, -49,117},
//    { -90,117,-127,90,-49,   0, 49, -90,127,-117, 90, -49,  0},
//    { -49, 49,-49,  49,-49,  49,-49,  49,-49,  49,-49,  49,-49}
//};

#endif /* SPEECH_DATA_H */