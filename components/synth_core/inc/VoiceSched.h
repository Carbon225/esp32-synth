#ifndef _SYNTH_CORE_VOICE_SCHED_H_
#define _SYNTH_CORE_VOICE_SCHED_H_

#include <array>
#include <Voice.h>

class VoiceSched
{
public:
    static constexpr int NUM_VOICES = 4;

    void pressKey(int key);

    void releaseKey(int key);

    std::array<Voice, NUM_VOICES>& voices();

private:
    std::array<Voice, NUM_VOICES> _voices;
};

#endif
