/* 
 * File:   speech_data.h
 * Author: Lenovo
 *
 * Created on April 18, 2026, 5:55 PM
 */

#ifndef SPEECH_DATA_H
#define	SPEECH_DATA_H

#include <avr/pgmspace.h>
#define N_MFCC 13
#define N_CLASSES 2

#ifdef	__cplusplus
extern "C" {
#endif
const int8_t PROGMEM mfcc_on[N_MFCC] = {-128, 127, 17, 39, 6, 10, -26, 10, 31, -15, -15, 4, -11};
const int8_t PROGMEM mfcc_off[N_MFCC] = {-128, 127, 29, 50, -12, -32, -57, 31, 33, -33, -5, 14, -19};
const int8_t PROGMEM mfcc_up[N_MFCC] = {-128, 127, 26, 10, -8, -1, -12, 14, 8, -29, 1, 2, -11};
const int8_t PROGMEM mfcc_down[N_MFCC] = {-128, 127, 15, 21, 1, 15, -17, 16, 14, -35, 3, 6, -16};
const int8_t PROGMEM mfcc_left[N_MFCC] = {-128, 123, -18, 61, -11, 29, -22, 23, 9, -19, -3, 3, 4};
const int8_t PROGMEM mfcc_right[N_MFCC] = {-128, 127, 13, 15, -18, 46, -18, 3, 2, -23, 6, -8, -11};
const int8_t PROGMEM mfcc_start[N_MFCC] = {-128, 127, 13, 26, -10, 4, -22, 21, 5, -39, 6, -2, -25};
const int8_t PROGMEM mfcc_stop[N_MFCC] = {-128, 127, 23, 21, 6, 8, -7, 14, 5, -20, 6, 4, -10};



const int8_t* const PROGMEM mfcc_table[] = {
    mfcc_on,
    mfcc_off,
    mfcc_up,
    mfcc_down,
    mfcc_right,
    mfcc_left,
    mfcc_start,
    mfcc_stop
           
};
const char* labels[] = {"on", "off", "up", "down", "right",  "left", "start" , "stop"};

#ifdef	__cplusplus
}
#endif

#endif	/* SPEECH_DATA_H */

