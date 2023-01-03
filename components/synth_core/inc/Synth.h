#ifndef _SYNTH_CORE_SYNTH_H_
#define _SYNTH_CORE_SYNTH_H_

#include <cmath>
#include <VoiceSched.h>

class Synth
{
public:
    void pressKey(int key)
    {
        _sched.pressKey(key);
    }

    void releaseKey(int key)
    {
        _sched.releaseKey(key);
    }

    float getSample()
    {
        float out = 0.f;

        for (auto &v : _sched.voices())
        {
            out += v.getSample();
        }

        return atanf(out);
    }

private:
    VoiceSched _sched;
};

#endif
