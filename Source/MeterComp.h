/*
 Meter components
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#pragma once


#define RED_LT -0.5f //dB
#define ORANGE_LT -3.0f //dB
#define YELLOW_LT -6.0f //dB
#define GREEN_LT -18.0f //dB

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiComp.h"
#include "MeterDecay.h"

//==============================================================================

class RoundLed : public Component {
public:
    
    RoundLed() {};
    
    Colour colour;
    
    void paint(Graphics &) override;
    
    void setColour(const Colour&);
    
    void resized() override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundLed)
};

//==============================================================================

class ActivityLed: public RoundLed, public Timer{
    
public:
    
    ActivityLed(){};
    
    void setColours(const Colour& colour1_,const Colour& colour2_);
    
    void toggle();
    
    class Callback {
    public:
        virtual ~Callback() = default;

        virtual bool isActive(int ledId) = 0;

    };
    
    void setCallback(Callback *cb, int ledId);
    
private:
    
    Colour colour1, colour2;
    
    int ledId;
    
    Callback *callback = nullptr;
    
    void timerCallback() override;
    
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
    
    static Colour dbToColor(float valDb);
    
    
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
    
    static Colour dbToColour(float valDb, float thDb);
    
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
