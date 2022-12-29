#include <Synth.h>


void Synth::pressKey(int key)
{
    _sched.pressKey(key);
}

void Synth::releaseKey(int key)
{
    _sched.releaseKey(key);
}

std::pair<float, float> Synth::getSample()
{
    auto out = std::make_pair(0.f, 0.f);

    for (auto &v : _sched.voices())
    {
        const auto &s = v.getSample();
        out.first += s.first;
        out.second += s.second;
    }

    return out;
}
