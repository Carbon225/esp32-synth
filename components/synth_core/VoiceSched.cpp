#include <VoiceSched.h>


void VoiceSched::pressKey(int key)
{
    auto best_voice = _voices.begin();

    for (auto voice = _voices.begin() + 1;
         voice != _voices.cend();
         ++voice)
    {
        if (voice->volume() < best_voice->volume())
        {
            best_voice = voice;
        }
    }

    best_voice->pressKey(key);
}

void VoiceSched::releaseKey(int key)
{
    for (auto &v : _voices)
    {
        if (v.isPressed() && v.pressedKey() == key)
        {
            v.releaseKey();
        }
    }
}

std::array<Voice, VoiceSched::NUM_VOICES> &VoiceSched::voices()
{
    return _voices;
}
