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

    std::pair<float, float> getSample();

    bool isPressed() const;

    int pressedKey() const;

    float volume() const;

private:
    float polyBlep(float t) const;
    float getDelayRight() const;
    float getDelayLeft() const;

    static constexpr int OSC_NUMBER = 1;

    // keys
    int _pressed_key = 0;

    // synth variables
    const float _master_volume = 1.f;

    // OSCs
    const float _dt = 1.f / 48000.f;
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
    const float _decay = 0.8f;
    const float _sustain = 0.f;

    float _amplitude = 0.f;
    bool _is_ascending = false;
};

#endif
