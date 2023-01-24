#ifndef ESP32_SYNTH_VOLUME_KNOB_H
#define ESP32_SYNTH_VOLUME_KNOB_H

#ifdef __cplusplus
extern "C" {
#endif


void volume_knob_init(void);

void volume_knob_override(int volume);

int volume_knob_get(void);


#ifdef __cplusplus
}
#endif

#endif
