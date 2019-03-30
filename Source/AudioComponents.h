/*
  ==============================================================================

    AudioComponents.h
    Created: 16 Mar 2019 11:44:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once
#define DB_MINUS_INF -100.0

#define RED_LT -0.5f //dB
#define YELLOW_LT -6.0f //dB
#define GREEN_LT -20.0f //dB

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class DecibelSlider : public Slider
{
public:
    
    DecibelSlider(){};
    
    double getValueFromText (const String& text) override
    {
       
        auto decibelText = text.upToFirstOccurrenceOf ("dB", false, false).trim();
        
        return decibelText.equalsIgnoreCase ("-INF") ? DB_MINUS_INF
        : decibelText.getDoubleValue();  // [2]
    }
    
    String getTextFromValue (double value) override
    {
        return Decibels::toString (value,1,DB_MINUS_INF);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecibelSlider)
};

//==============================================================================

class FrequencySlider : public Slider
{
public:
    
    FrequencySlider(){};
    
    double getValueFromText (const String& text) override
    {
        
        auto hzText = text.upToFirstOccurrenceOf ("Hz", false, false).trim();
        auto hzVal = roundToInt(hzText.getDoubleValue());
        return hzVal;
    }
    
    String getTextFromValue (double value) override
    {
        std::ostringstream valueString;
        valueString << roundToInt(value) << " Hz";
        return valueString.str();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencySlider)
};

//==============================================================================

class RoundLed : public Component
{
public:
    
    RoundLed(){};
    
    Colour colour;
    
    void paint(Graphics&) override;
    void resized() override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoundLed)
};

//==============================================================================
class MultiChannelLedBar : public Component, public Timer
{
public:
    
    MultiChannelLedBar(){};
    ~MultiChannelLedBar(){};
    void paint(Graphics&) override;
    void resized() override;

    void setSource(const std::vector<float> &source,SpinLock &lock);
    void setHorizontal(){isHorizontal = true;};
    void setVertical(){isHorizontal = false;};
    

private:
    
    bool isHorizontal = true;
    size_t num=0;
    const std::vector<float> *source;
    SpinLock *lock;
    std::vector<std::unique_ptr<RoundLed>> leds;
    
    void timerCallback() override;
    
    void makeLayout();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiChannelLedBar)
};

//==============================================================================
class SingleChannelLedBar : public Component, public Timer
{
public:
    
    SingleChannelLedBar(size_t numLeds = 7, bool isHorizontal = false);
    ~SingleChannelLedBar(){};

    void setSource(const std::vector<float> &source,int ch, SpinLock &lock);
    void paint(Graphics&) override;
    void resized() override;
    
private:
    
    bool isHorizontal;
    size_t num;
    
    const std::vector<float> *source;
    int ch;
    SpinLock *lock;
    
    std::vector<float> th;
    std::vector<std::unique_ptr<RoundLed>> leds;
    
    static Colour thToColour(float th, bool active);
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleChannelLedBar)
};
