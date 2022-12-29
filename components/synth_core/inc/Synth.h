#ifndef _SYNTH_CORE_SYNTH_H_
#define _SYNTH_CORE_SYNTH_H_

#include <utility>
#include <VoiceSched.h>

class Synth
{
public:
    void pressKey(int key);

    void releaseKey(int key);

    std::pair<float, float> getSample();

private:
    VoiceSched _sched;
};

#endif
