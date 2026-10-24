#pragma once
// Reine DSP-Mathematik, bewusst OHNE JUCE-Abhängigkeit (testbar mit tests/dsp_test.cpp).
#include <algorithm>
#include <cmath>

namespace rc
{
    enum Shape { Tanh = 0, Cubic = 1, Sine = 2, Warm = 3 };

    constexpr float kHalfPi = 1.57079632679f;

    inline float dbToGain (float db) noexcept { return std::pow (10.0f, db * 0.05f); }

    // Normierte Sättigungskurven: Steigung 1 bei 0, Maximum exakt 1.
    inline float shapeFn (int shape, float u) noexcept
    {
        switch (shape)
        {
            case Cubic:  // polynomial, endet sanft (Steigung 0) bei u = 1.5
            {
                if (std::fabs (u) >= 1.5f) return std::copysign (1.0f, u);
                return u - (4.0f / 27.0f) * u * u * u;
            }
            case Sine:   // Sinus-Viertelwelle, endet sanft bei u = pi/2
            {
                if (std::fabs (u) >= kHalfPi) return std::copysign (1.0f, u);
                return std::sin (u);
            }
            case Warm:   // asymmetrisch: negative Halbwelle sättigt früher -> gerade Harmonische
                return u >= 0.0f ? std::tanh (u) : 0.8f * std::tanh (u / 0.8f);
            case Tanh:
            default:
                return std::tanh (u);
        }
    }

    // x ist bereits auf Ceiling = 1.0 normiert. t = Schwelle, unterhalb der das Signal
    // unangetastet bleibt (linear). Darüber geht die Kurve stetig differenzierbar in die
    // gewählte Sättigung über und erreicht exakt +-1.
    inline float softClip (int shape, float x, float t) noexcept
    {
        const float k = 1.0f - t;
        if (x >  t) return  t + k * shapeFn (shape, (x - t) / k);
        if (x < -t) return -t + k * shapeFn (shape, (x + t) / k);
        return x;
    }

    inline float edgeToThreshold (float edge) noexcept   // 0..1 -> 0..0.85
    {
        return 0.85f * std::min (1.0f, std::max (0.0f, edge));
    }

    struct Gains { float pre; float post; };

    // pre: Drive + auf Ceiling normieren. post: zurück auf Ceiling, Output-Gain,
    // optional Drive wieder herausrechnen (Auto-Gain = fairer A/B-Vergleich).
    inline Gains computeGains (float driveDb, float ceilingDb, float outputDb, bool autoGain) noexcept
    {
        const float drive = dbToGain (driveDb);
        const float ceil  = dbToGain (ceilingDb);
        const float out   = dbToGain (outputDb);
        return { drive / ceil, ceil * out * (autoGain ? 1.0f / drive : 1.0f) };
    }
}
