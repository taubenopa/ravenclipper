#include "PluginEditor.h"

using namespace juce;

//==============================================================================
RavenLook::RavenLook()
{
    using namespace raven;
    setColour (ComboBox::backgroundColourId, Colour (0xff15132a));
    setColour (ComboBox::outlineColourId, violet.withAlpha (0.45f));
    setColour (ComboBox::textColourId, silver);
    setColour (ComboBox::arrowColourId, violet);
    setColour (PopupMenu::backgroundColourId, Colour (0xff15132a));
    setColour (PopupMenu::textColourId, silver);
    setColour (PopupMenu::highlightedBackgroundColourId, violet.withAlpha (0.55f));
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (ToggleButton::textColourId, silver);
    setColour (ToggleButton::tickColourId, gold);
    setColour (ToggleButton::tickDisabledColourId, silver.withAlpha (0.4f));
    setColour (Slider::textBoxTextColourId, silver);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Label::textColourId, silver);
}

void RavenLook::drawRotarySlider (Graphics& g, int x, int y, int w, int h,
                                  float pos, float startAngle, float endAngle, Slider&)
{
    using namespace raven;
    const auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.0f);
    const float r  = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  c  = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);

    // Knopfkörper (dunkles Gefieder mit Glanz)
    const float rb = r * 0.76f;
    g.setGradientFill (ColourGradient (Colour (0xff302c4f), c.x, c.y - rb, Colour (0xff0b0a15), c.x, c.y + rb, false));
    g.fillEllipse (c.x - rb, c.y - rb, rb * 2.0f, rb * 2.0f);
    g.setColour (violet.withAlpha (0.25f));
    g.drawEllipse (c.x - rb, c.y - rb, rb * 2.0f, rb * 2.0f, 1.0f);

    // Spur + Wert-Bogen
    Path track, value;
    track.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, endAngle, true);
    value.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, angle, true);
    g.setColour (Colours::white.withAlpha (0.09f));
    g.strokePath (track, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (violet.withAlpha (0.30f));
    g.strokePath (value, PathStrokeType (7.0f, PathStrokeType::curved, PathStrokeType::rounded));  // Glow
    g.setColour (violet);
    g.strokePath (value, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));

    // Zeiger: kleine Feder
    Path feather;
    feather.startNewSubPath (0.0f, -rb * 0.92f);
    feather.quadraticTo (rb * 0.14f, -rb * 0.55f, 0.0f, -rb * 0.18f);
    feather.quadraticTo (-rb * 0.14f, -rb * 0.55f, 0.0f, -rb * 0.92f);
    feather.closeSubPath();
    g.setColour (silver);
    g.fillPath (feather, AffineTransform::rotation (angle).translated (c.x, c.y));
}

//==============================================================================
void CurveView::paint (Graphics& g)
{
    using namespace raven;
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (Colour (0xff0d0c18).withAlpha (0.92f));
    g.fillRoundedRectangle (b, 10.0f);
    g.setColour (Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (b, 10.0f, 1.0f);

    g.setColour (silver.withAlpha (0.55f));
    g.setFont (Font (FontOptions (10.5f, Font::bold)));
    g.drawText ("TRANSFER CURVE", (int) b.getX() + 12, (int) b.getY() + 6, 160, 14, Justification::left);

    const auto plot = b.reduced (16.0f).withTrimmedTop (8.0f);
    constexpr float range = 1.25f;
    auto mapX = [&] (float v) { return plot.getX() + (v + 1.0f) * 0.5f * plot.getWidth(); };
    auto mapY = [&] (float v) { return plot.getBottom() - (v + range) / (2.0f * range) * plot.getHeight(); };

    // Achsen, Ceiling-Linien, Diagonale (= unbearbeitet)
    g.setColour (Colours::white.withAlpha (0.10f));
    g.drawLine (mapX (0.0f), plot.getY(), mapX (0.0f), plot.getBottom());
    g.drawLine (plot.getX(), mapY (0.0f), plot.getRight(), mapY (0.0f));
    g.setColour (gold.withAlpha (0.22f));
    g.drawLine (plot.getX(), mapY (1.0f),  plot.getRight(), mapY (1.0f));
    g.drawLine (plot.getX(), mapY (-1.0f), plot.getRight(), mapY (-1.0f));
    g.setColour (Colours::white.withAlpha (0.12f));
    g.drawLine (mapX (-1.0f), mapY (-1.0f), mapX (1.0f), mapY (1.0f));

    const int  shape = (int) apvts.getRawParameterValue ("shape")->load();
    const auto gains = rc::computeGains (apvts.getRawParameterValue ("drive")->load(),
                                         apvts.getRawParameterValue ("ceiling")->load(),
                                         apvts.getRawParameterValue ("output")->load(),
                                         apvts.getRawParameterValue ("autogain")->load() > 0.5f);
    const float t = rc::edgeToThreshold (apvts.getRawParameterValue ("edge")->load());

    Path curve;
    const int steps = jmax (2, (int) plot.getWidth());
    for (int i = 0; i <= steps; ++i)
    {
        const float xin = -1.0f + 2.0f * (float) i / (float) steps;
        const float y   = jlimit (-range, range, rc::softClip (shape, xin * gains.pre, t) * gains.post);
        if (i == 0) curve.startNewSubPath (mapX (xin), mapY (y));
        else        curve.lineTo (mapX (xin), mapY (y));
    }
    g.setColour (violet.withAlpha (0.28f));
    g.strokePath (curve, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (violet);
    g.strokePath (curve, PathStrokeType (2.0f, PathStrokeType::curved, PathStrokeType::rounded));
}

//==============================================================================
RavenClipperAudioProcessorEditor::RavenClipperAudioProcessorEditor (RavenClipperAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), curve (p.apvts)
{
    setLookAndFeel (&look);

    addAndMakeVisible (ravens);   // zuerst hinzugefügt = liegt hinter den Reglern
    addAndMakeVisible (curve);

    auto setupKnob = [this] (Knob& k, const String& name)
    {
        k.slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (Slider::TextBoxBelow, false, 78, 18);
        k.slider.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
        k.label.setText (name, dontSendNotification);
        k.label.setJustificationType (Justification::centred);
        k.label.setFont (Font (FontOptions (11.5f, Font::bold)));
        k.label.setColour (Label::textColourId, raven::silver.withAlpha (0.8f));
        addAndMakeVisible (k.slider);
        addAndMakeVisible (k.label);
    };
    setupKnob (driveK, "DRIVE");
    setupKnob (edgeK,  "EDGE");
    setupKnob (ceilK,  "CEILING");
    setupKnob (mixK,   "MIX");
    setupKnob (outK,   "OUTPUT");

    auto setupLabel = [this] (Label& l, const String& text)
    {
        l.setText (text, dontSendNotification);
        l.setFont (Font (FontOptions (11.5f, Font::bold)));
        l.setColour (Label::textColourId, raven::silver.withAlpha (0.8f));
        addAndMakeVisible (l);
    };
    setupLabel (shapeL, "SHAPE");
    setupLabel (osL,    "OVERSAMPLING");

    shapeBox.addItemList (p.apvts.getParameter ("shape")->getAllValueStrings(), 1);
    osBox.addItemList    (p.apvts.getParameter ("os")->getAllValueStrings(), 1);
    addAndMakeVisible (shapeBox);
    addAndMakeVisible (osBox);

    autoBtn.setButtonText ("Auto Gain (compensates Drive)");
    addAndMakeVisible (autoBtn);

    driveA = std::make_unique<SA> (p.apvts, "drive",   driveK.slider);
    edgeA  = std::make_unique<SA> (p.apvts, "edge",    edgeK.slider);
    ceilA  = std::make_unique<SA> (p.apvts, "ceiling", ceilK.slider);
    mixA   = std::make_unique<SA> (p.apvts, "mix",     mixK.slider);
    outA   = std::make_unique<SA> (p.apvts, "output",  outK.slider);
    shapeA = std::make_unique<CA> (p.apvts, "shape",   shapeBox);
    osA    = std::make_unique<CA> (p.apvts, "os",      osBox);
    autoA  = std::make_unique<BA> (p.apvts, "autogain", autoBtn);

    // Doppelklick = Standardwert
    auto defaultOf = [&p] (const char* id)
    {
        auto* prm = p.apvts.getParameter (id);
        return (double) prm->convertFrom0to1 (prm->getDefaultValue());
    };
    driveK.slider.setDoubleClickReturnValue (true, defaultOf ("drive"));
    edgeK.slider.setDoubleClickReturnValue  (true, defaultOf ("edge"));
    ceilK.slider.setDoubleClickReturnValue  (true, defaultOf ("ceiling"));
    mixK.slider.setDoubleClickReturnValue   (true, defaultOf ("mix"));
    outK.slider.setDoubleClickReturnValue   (true, defaultOf ("output"));

    setSize (720, 400);
    startTimerHz (60);
}

RavenClipperAudioProcessorEditor::~RavenClipperAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void RavenClipperAudioProcessorEditor::timerCallback()
{
    auto norm = [] (float lin)
    {
        const float db = Decibels::gainToDecibels (lin, -60.0f);
        return jlimit (0.0f, 1.0f, jmap (db, -48.0f, 0.0f, 0.0f, 1.0f));
    };

    inLvl     = jmax (norm (proc.inPeak.exchange  (0.0f)), inLvl  * 0.88f);
    outLvl    = jmax (norm (proc.outPeak.exchange (0.0f)), outLvl * 0.88f);
    agitation = jmax (jlimit (0.0f, 1.0f, proc.activity.exchange (0.0f) * 3.0f), agitation * 0.96f);

    ravens.tick (1.0f / 60.0f, agitation);
    repaint();
}

void RavenClipperAudioProcessorEditor::paint (Graphics& g)
{
    using namespace raven;
    const auto w = (float) getWidth(), h = (float) getHeight();

    // Nachthimmel
    g.setGradientFill (ColourGradient (indigo, 0.0f, 0.0f, night, 0.0f, h, false));
    g.fillAll();

    // Mond + Schein
    const Point<float> moon (w - 70.0f, 40.0f);
    g.setGradientFill (ColourGradient (silver.withAlpha (0.30f), moon.x, moon.y,
                                       silver.withAlpha (0.0f), moon.x + 110.0f, moon.y, true));
    g.fillEllipse (moon.x - 110.0f, moon.y - 110.0f, 220.0f, 220.0f);
    g.setColour (Colour (0xffe9e5ff).withAlpha (0.93f));
    g.fillEllipse (moon.x - 22.0f, moon.y - 22.0f, 44.0f, 44.0f);
    g.setColour (Colour (0xffbab4dc).withAlpha (0.55f));
    g.fillEllipse (moon.x - 10.0f, moon.y - 12.0f, 11.0f, 11.0f);
    g.fillEllipse (moon.x + 3.0f,  moon.y + 4.0f,  8.0f,  8.0f);

    // Titel
    g.setColour (silver);
    g.setFont (Font (FontOptions (30.0f, Font::bold)));
    g.drawText ("RAVENCLIPPER", 24, 12, 380, 34, Justification::left);
    g.setColour (violet);
    g.setFont (Font (FontOptions (11.0f, Font::bold)));
    g.drawText ("SOFT CLIPPER FOR BEATS", 26, 45, 300, 14, Justification::left);

    // Pegel: oben Input, unten Output
    auto drawMeter = [&] (const String& name, float y, float level)
    {
        g.setColour (silver.withAlpha (0.6f));
        g.setFont (Font (FontOptions (10.5f, Font::bold)));
        g.drawText (name, 24, (int) y - 4, 34, 14, Justification::left);

        const Rectangle<float> bar (60.0f, y, w - 60.0f - 24.0f, 7.0f);
        g.setColour (Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (bar, 3.5f);
        g.setGradientFill (ColourGradient (violet, bar.getX(), 0.0f, gold, bar.getRight(), 0.0f, false));
        g.fillRoundedRectangle (bar.withWidth (jmax (0.0f, bar.getWidth() * level)), 3.5f);
    };
    drawMeter ("IN",  326.0f, inLvl);
    drawMeter ("OUT", 344.0f, outLvl);

    g.setColour (silver.withAlpha (0.35f));
    g.setFont (Font (FontOptions (10.5f)));
    g.drawText ("RavenClipper 1.0  -  free & open source", 24, 372, (int) w - 48, 14, Justification::left);
}

void RavenClipperAudioProcessorEditor::resized()
{
    ravens.setBounds (getLocalBounds());
    curve.setBounds (24, 84, 220, 220);

    Knob* knobs[] = { &driveK, &edgeK, &ceilK, &mixK, &outK };
    for (int i = 0; i < 5; ++i)
    {
        const int x = 268 + i * 86;
        knobs[i]->label.setBounds  (x, 80, 84, 16);
        knobs[i]->slider.setBounds (x, 98, 84, 106);
    }

    shapeL.setBounds   (268, 216, 196, 16);
    shapeBox.setBounds (268, 234, 196, 28);
    osL.setBounds      (488, 216, 196, 16);
    osBox.setBounds    (488, 234, 196, 28);
    autoBtn.setBounds  (268, 274, 300, 28);
}
