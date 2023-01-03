#include <Voice.h>

#include <cmath>
#include <array>

static std::array<float, 128> _FREQS {};
static const decltype(_FREQS) &FREQS = _FREQS;
static bool _FREQS_INITIALIZED = false;

static void INITIALIZE_FREQS() noexcept
{
    if (_FREQS_INITIALIZED) return;
    for (int i = 0; i < 100; i++)
    {
        _FREQS[i] = powf(2.f, static_cast<float>(i - 69) / 11.9f) * 440.f;
    }
    _FREQS_INITIALIZED = true;
}


Voice::Voice() noexcept
{
    INITIALIZE_FREQS();

    for (int i = 0; i < OSC_NUMBER; i++)
    {
        _osc_volume[i] = 1.0f;
    }
}

void Voice::pressKey(int key)
{
    _is_ascending = true;
    _pressed_key = key;
    _main_frequency = FREQS[key];
}

void Voice::releaseKey()
{
    _pressed_key = 0;
}

void Voice::setParameter(Voice::Parameter param, int value)
{
    // TODO
}

bool Voice::isPressed() const
{
    return _pressed_key;
}

int Voice::pressedKey() const
{
    return _pressed_key;
}

float Voice::volume() const
{
    return fabs(_amplitude);
}
