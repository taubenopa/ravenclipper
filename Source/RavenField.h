#pragma once
#include <JuceHeader.h>

// Animierte Raben-Silhouetten (rein vektorbasiert, keine Bilddateien nötig).
// Mehr Clipping im Audio -> die Raben fliegen schneller und schlagen kräftiger mit den Flügeln.
class RavenField : public juce::Component
{
public:
    RavenField()
    {
        setInterceptsMouseClicks (false, false);
        setOpaque (false);
    }

    void resized() override
    {
        if (! ravens.empty() || getWidth() <= 0)
            return;

        juce::Random r0 (1337);
        for (int i = 0; i < 7; ++i)
        {
            Raven r;
            r.scale    = 0.70f + r0.nextFloat() * 0.90f;
            r.x        = r0.nextFloat() * (float) getWidth();
            r.baseY    = 28.0f + r0.nextFloat() * juce::jmax (10.0f, (float) getHeight() - 90.0f);
            r.speed    = 26.0f + r0.nextFloat() * 34.0f;
            r.flapRate = 1.5f + r0.nextFloat() * 0.9f;
            r.phase    = r0.nextFloat() * juce::MathConstants<float>::twoPi;
            r.bobPhase = r0.nextFloat() * juce::MathConstants<float>::twoPi;
            r.bobAmp   = 4.0f + r0.nextFloat() * 8.0f;
            r.alpha    = 0.55f + 0.45f * (r.scale - 0.7f) / 0.9f;   // größer = näher = deckender
            ravens.push_back (r);
        }
    }

    // dt in Sekunden, agitation 0..1
    void tick (float dt, float agitation)
    {
        constexpr float twoPi = juce::MathConstants<float>::twoPi;
        smoothAgitation += (agitation - smoothAgitation) * 0.12f;

        const float w = (float) getWidth(), h = (float) getHeight();
        for (auto& r : ravens)
        {
            r.x       += r.speed * (1.0f + smoothAgitation * 1.6f) * dt;
            r.phase    = std::fmod (r.phase + r.flapRate * (1.0f + smoothAgitation * 1.3f) * dt * twoPi, twoPi);
            r.bobPhase = std::fmod (r.bobPhase + dt * 0.9f, twoPi);

            if (r.x > w + 70.0f)
            {
                r.x     = -70.0f;
                r.baseY = 28.0f + rng.nextFloat() * juce::jmax (10.0f, h - 90.0f);
            }
        }
    }

    void paint (juce::Graphics& g) override
    {
        for (const auto& r : ravens)
            drawRaven (g, r);
    }

private:
    struct Raven { float x = 0, baseY = 0, speed = 30, scale = 1, flapRate = 2, phase = 0,
                         bobPhase = 0, bobAmp = 6, alpha = 1; };

    // Flügel: flap = +1 ganz oben, -1 ganz unten
    static juce::Path makeWing (float flap, float len)
    {
        juce::Path p;
        const float tipY = -flap * len * 0.95f - 2.0f;
        p.startNewSubPath (6.0f, -3.0f);
        p.quadraticTo (2.0f, -flap * len * 0.60f - 10.0f, -len * 0.35f, tipY);
        p.quadraticTo (-len * 0.50f, -flap * len * 0.20f + 7.0f, -9.0f, 2.0f);
        p.closeSubPath();
        return p;
    }

    void drawRaven (juce::Graphics& g, const Raven& r) const
    {
        const float flap = std::sin (r.phase);
        const float bob  = std::sin (r.bobPhase) * r.bobAmp;

        juce::Graphics::ScopedSaveState state (g);
        g.addTransform (juce::AffineTransform::rotation (-0.10f * flap)
                            .scaled (r.scale)
                            .translated (r.x, r.baseY + bob));

        const juce::Colour dark (0xff07060d), rim (0xff8b6cff);
        auto draw = [&] (const juce::Path& p, float a)
        {
            g.setColour (rim.withAlpha (0.35f * r.alpha * a));      // Mondlicht-Kante
            g.strokePath (p, juce::PathStrokeType (1.2f));
            g.setColour (dark.withAlpha (r.alpha * a));
            g.fillPath (p);
        };

        juce::Path body, tail, head, beak;
        body.addEllipse (-14.0f, -5.0f, 28.0f, 10.0f);
        tail.addTriangle (-12.0f, -2.0f, -29.0f, -5.0f, -26.0f, 3.5f);
        head.addEllipse (10.0f, -8.5f, 10.0f, 9.0f);
        beak.addTriangle (18.5f, -6.5f, 29.0f, -3.0f, 18.5f, -1.5f);

        draw (makeWing (std::sin (r.phase - 0.5f), 30.0f), 0.65f);   // hinterer Flügel
        draw (tail, 1.0f);
        draw (body, 1.0f);
        draw (head, 1.0f);
        draw (beak, 1.0f);
        draw (makeWing (flap, 34.0f), 1.0f);                          // vorderer Flügel

        g.setColour (juce::Colour (0xffffc247).withAlpha (r.alpha));  // goldenes Auge
        g.fillEllipse (15.2f, -5.6f, 2.0f, 2.0f);
    }

    std::vector<Raven> ravens;
    juce::Random rng { 42 };
    float smoothAgitation = 0.0f;
};
