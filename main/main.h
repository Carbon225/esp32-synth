#ifndef ESP32_SYNTH_MAIN_H
#define ESP32_SYNTH_MAIN_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


void synth_note_on(uint8_t note, uint8_t vel);

void synth_note_off(uint8_t note);


#ifdef __cplusplus
}
#endif


#endif
