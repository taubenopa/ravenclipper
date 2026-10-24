// Schneller Mathe-Test für den Clipper-Kern.  g++ -std=c++17 -I Source tests/dsp_test.cpp && ./a.out
#include "ClipperDSP.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; std::printf("FAIL: " __VA_ARGS__); std::printf("\n"); } } while (0)

int main()
{
    const float edges[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

    for (int shape = 0; shape < 4; ++shape)
        for (float e : edges)
        {
            const float t = rc::edgeToThreshold (e);
            float prev = rc::softClip (shape, -20.0f, t);
            float maxAbs = 0.0f;

            for (float x = -20.0f; x <= 20.0f; x += 0.0005f)
            {
                const float y = rc::softClip (shape, x, t);
                maxAbs = std::max (maxAbs, std::fabs (y));
                CHECK (y >= prev - 1e-6f, "nicht monoton: shape %d edge %.2f x %.4f", shape, e, x);
                CHECK (std::fabs (y - prev) < 0.01f, "Sprung (C0): shape %d edge %.2f x %.4f", shape, e, x);
                prev = y;
            }
            CHECK (maxAbs <= 1.0f + 1e-6f, "Ceiling ueberschritten: shape %d edge %.2f max %.6f", shape, e, maxAbs);
            CHECK (maxAbs > 0.79f,          "Ceiling nie erreicht: shape %d edge %.2f max %.6f", shape, e, maxAbs);

            const float small = 0.01f;   // kleine Signale muessen linear (Steigung 1) durchgehen
            CHECK (std::fabs (rc::softClip (shape, small, t) - small) < 1e-4f, "Kleinsignal verzerrt: shape %d", shape);
        }

    // Auto-Gain: kleines Signal bleibt bei beliebigem Drive/Ceiling auf Unity (Output 0 dB)
    for (float drive : { 0.0f, 6.0f, 18.0f })
        for (float ceil : { -6.0f, -1.0f, 0.0f })
        {
            const auto g = rc::computeGains (drive, ceil, 0.0f, true);
            CHECK (std::fabs (g.pre * g.post - 1.0f) < 1e-4f, "Auto-Gain nicht unity: drive %.1f ceil %.1f", drive, ceil);
        }

    // Ceiling-Garantie am Ausgang: egal wie laut der Input, Peak <= Ceiling (ohne Output-Gain)
    for (float ceilDb : { -6.0f, -3.0f, -0.3f })
    {
        const auto g = rc::computeGains (12.0f, ceilDb, 0.0f, false);
        float peak = 0.0f;
        for (float x = -4.0f; x <= 4.0f; x += 0.001f)
            peak = std::max (peak, std::fabs (rc::softClip (rc::Tanh, x * g.pre, 0.3f) * g.post));
        CHECK (peak <= rc::dbToGain (ceilDb) + 1e-5f, "Ceiling %.1f dB gerissen: peak %.5f", ceilDb, peak);
    }

    std::printf (failures == 0 ? "ALLE DSP-TESTS OK\n" : "%d Fehler\n", failures);
    return failures == 0 ? 0 : 1;
}
