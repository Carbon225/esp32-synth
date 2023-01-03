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

std::pair<float, float> Voice::getSample()
{
    // oscs____________________________________________________________________________________________________________________
    float mixer_output = 0.0f;
    _temp_detune = 1.f;
    for (int i = 0; i < OSC_NUMBER; i++)
    {
        _phases[i] += _main_frequency * _temp_detune * _osc_ratio[i] * _dt;
        if (_phases[i] > 1.f)
            _phases[i] -= 1.f;
        mixer_output += (_phases[i] * 2.f - 1.f - polyBlep(_phases[i])) * _osc_volume[i];
        _temp_detune *= _detune;
    }

    // filter__________________________________________________________________________________________________________________
    _external_cutoff = _amplitude * _eg_int;
    _smooth_cutoff += 0.0005f * (_cutoff - _smooth_cutoff);

    float final_cutoff = (_smooth_cutoff + _external_cutoff) * (_smooth_cutoff + _external_cutoff);

    if (final_cutoff > 0.99f)
        final_cutoff = 0.99f;

    _buf[0] += final_cutoff * (mixer_output - _buf[0] + (resonance + resonance / (1.f - final_cutoff)) * (_buf[0] - _buf[1]));
    _buf[1] += final_cutoff * (_buf[0] - _buf[1]);
    _buf[2] += final_cutoff * (_buf[1] - _buf[2]);
    _buf[3] += final_cutoff * (_buf[2] - _buf[3]);

    // envelope generator______________________________________________________________________________________________________
    if (_pressed_key)
    {
        if (_is_ascending)
        {
            _amplitude += _dt / _attack;
            if (_amplitude >= 1.f)
                _is_ascending = false;
        }

        else
        {
            _amplitude -= _dt / _decay;
            if (_amplitude < _sustain)
                _amplitude = _sustain;
        }
    }

    else
    {
        _is_ascending = true;
        _amplitude -= _dt / _decay;
        if (_amplitude < 0.f)
            _amplitude = 0.f;
    }

    float audio_output = 0.1f * _buf[3] * _amplitude * _master_volume;

    return {
        atanf(audio_output + getDelayLeft()),
        atanf(audio_output + getDelayRight())
    };
}

float Voice::polyBlep(float t) const
{
    float dx = _main_frequency * _temp_detune * _dt;

    if (t < dx)
    {
        t /= dx;
        return t + t - t * t - 1.f;
    }
    else if (t > 1.f - dx)
    {
        t = (t - 1.f) / dx;
        return t * t + t + t + 1.f;
    }
    else
    {
        return 0.f;
    }
}

float Voice::getDelayRight() const
{
    return 0.f;
}

float Voice::getDelayLeft() const
{
    return 0.f;
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
