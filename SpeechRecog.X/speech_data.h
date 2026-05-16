#ifndef WORD_FEATURES_H
#define WORD_FEATURES_H

#include <avr/pgmspace.h>

#define N_CLASSES 4
#define N_FRAMES  8


// ---- ZCR reference arrays (one per word) ----
static const float down_zcr [N_FRAMES] PROGMEM = {120.85, 144.05, 124.45, 159.75, 158.85, 116.25,  88.00,  98.85};
static const float left_zcr [N_FRAMES] PROGMEM = {113.50, 151.15, 188.65, 247.75, 204.20, 148.35, 154.25, 152.85};
static const float right_zcr[N_FRAMES] PROGMEM = {120.90, 145.95, 167.80, 230.35, 202.35, 147.65, 175.10, 125.80};
static const float up_zcr   [N_FRAMES] PROGMEM = { 90.90, 103.05, 122.90, 120.15,  94.90,  73.35,  57.15,  62.60};

// ---- STE reference arrays (one per word) ----
static const float down_ste [N_FRAMES] PROGMEM = {124665.15, 264618.45,  393496.15, 1324715.60, 1495920.65, 884985.65, 212844.85,  85533.85};
static const float left_ste [N_FRAMES] PROGMEM = {257887.05, 286715.65,  707708.00, 1112791.10,  792734.75, 185814.20, 139681.80, 108527.30};
static const float right_ste[N_FRAMES] PROGMEM = {187159.25, 300386.60,  682900.45, 1591138.50, 1406012.90, 515662.45, 217630.95,  94189.80};
static const float up_ste   [N_FRAMES] PROGMEM = {187139.65, 328982.05,  615536.85,  959417.75,  559720.60, 488151.50, 497501.75, 219518.95};

// ---- Pointer tables indexed by class ----
static const float * const zcr_table[N_CLASSES] PROGMEM = {down_zcr, left_zcr, right_zcr, up_zcr};
static const float * const ste_table[N_CLASSES] PROGMEM = {down_ste, left_ste, right_ste, up_ste};

// ---- Labels ----
static const char * const LABELS[N_CLASSES] PROGMEM = {"down", "left", "right", "up"};

#endif // WORD_FEATURES_H