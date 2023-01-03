#ifndef _SYNTH_CORE_VOICE_SCHED_H_
#define _SYNTH_CORE_VOICE_SCHED_H_

#include <array>
#include <Voice.h>

class VoiceSched
{
public:
    static constexpr int NUM_VOICES = 8;

    void pressKey(int key, int vel)
    {
        auto best_voice = _voices.begin();

        for (auto voice = _voices.begin() + 1;
            voice != _voices.cend();
            ++voice)
        {
            if (!voice->isPressed())
            {
                best_voice = voice;
                break;
            }

            if (voice->pressedKey() < best_voice->pressedKey())
            {
                best_voice = voice;
            }

            // if (voice->volume() < best_voice->volume())
            // {
            //     best_voice = voice;
            // }
        }

        best_voice->pressKey(key, vel);
    }

    void releaseKey(int key)
    {
        for (auto &v : _voices)
        {
            if (v.isPressed() && v.pressedKey() == key)
            {
                v.releaseKey();
            }
        }
    }

    std::array<Voice, NUM_VOICES>& voices()
    {
        return _voices;
    }

private:
    std::array<Voice, NUM_VOICES> _voices;
};

#endif
