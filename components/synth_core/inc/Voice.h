#ifndef _SYNTH_CORE_VOICE_H_
#define _SYNTH_CORE_VOICE_H_

#include <utility>
#include <array>
#include <cstdint>

class Voice
{
public:
    Voice() noexcept;

    enum class Parameter
    {
        OSC2_TRANSPOSE,
        OSC2_DETUNE,
        OSC2_VOLUME,
        CUTOFF,
        RESONANCE,
        CUTOFF_FROM_EQ,
        ATTACK,
        SUSTAIN,
        DECAY_RELEASE,
        DECAY_FEEDBACK,
    };

    void pressKey(int key);

    void releaseKey();

    void setParameter(Parameter param, int value);

    float getSample()
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

        return audio_output;
    }

    bool isPressed() const;

    int pressedKey() const;

    float volume() const;

private:
    float polyBlep(float t) const
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

    static constexpr int OSC_NUMBER = 1;

    // keys
    int _pressed_key = 0;

    // synth variables
    const float _master_volume = 1.f;

    // OSCs
    const float _dt = 1.f / 44100.f;
    float _main_frequency = 0.f;
    std::array<float, OSC_NUMBER> _phases {};
    const std::array<float, OSC_NUMBER> _osc_ratio {1.0f};
    const float _detune = 1.005f;
    std::array<float, OSC_NUMBER> _osc_volume {};
    float _temp_detune = 1.f;

    // LPF
    const float _cutoff = 0.9f;
    const float resonance = 0.f;
    const float _eg_int = 0.f;

    std::array<float, 4> _buf = { 0 };
    float _smooth_cutoff = 0.f;
    float _external_cutoff = 0.f;

    // envelope generator
    const float _attack = 0.01f;
    const float _decay = 0.6f;
    const float _sustain = 0.f;

    float _amplitude = 0.f;
    bool _is_ascending = false;
};

#endif
