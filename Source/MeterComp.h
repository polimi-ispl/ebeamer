/*
 Meter components
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once


#define RED_LT -0.5f //dB
#define YELLOW_LT -6.0f //dB
#define GREEN_LT -20.0f //dB

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiComp.h"
#include "MeterDecay.h"

//==============================================================================

class RoundLed : public Component {
public:

    RoundLed() {};

    Colour colour;

    void paint(Graphics &) override;

    void resized() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundLed)
};


//==============================================================================
class MultiChannelLedBar : public Component, public Timer {
public:

    MultiChannelLedBar() {};

    ~MultiChannelLedBar() {};

    void paint(Graphics &) override;

    void resized() override;

    void setCallback(MeterDecay::Callback *cb, int metId);

    void setHorizontal() { isHorizontal = true; };

    void setVertical() { isHorizontal = false; };


private:

    MeterDecay::Callback *callback = nullptr;
    int meterId;

    bool isHorizontal = true;

    std::vector<std::unique_ptr<RoundLed>> leds;
    std::vector<float> values;

    void timerCallback() override;

    void makeLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChannelLedBar)
};

//==============================================================================
class SingleChannelLedBar : public Component, public Timer {
public:

    SingleChannelLedBar(size_t numLeds = 7, bool isHorizontal = false);

    ~SingleChannelLedBar() {};

    void setCallback(MeterDecay::Callback *cb, int meterId, int channel);

    void paint(Graphics &) override;

    void resized() override;

    static Colour thToColour(float th, bool active);

private:

    MeterDecay::Callback *provider = nullptr;
    int meterId;
    int channel;

    bool isHorizontal;

    std::vector<float> th;
    std::vector<std::unique_ptr<RoundLed>> leds;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleChannelLedBar)
};
