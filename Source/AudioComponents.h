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

class PanSlider : public Slider
{
public:
    
    PanSlider(){};
    
    double getValueFromText (const String& text) override
    {
        
        if (text.startsWithIgnoreCase("C") || text.startsWithIgnoreCase("0"))
        {
            return 0;
        }
        
        float sign = 1;
        if (text.endsWithIgnoreCase("L")){
            sign = -1;
        }
        
        String valueText = text.upToFirstOccurrenceOf ("%", false, false).trim();
        double val = valueText.getDoubleValue()/100.;
        return sign*val;
    }
    
    String getTextFromValue (double value) override
    {
        jassert(value >= -1 && value <= 1);
        std::ostringstream valueString;
        if (value == 0){
            valueString << "C";
        }
        else if (value < 0)
        {
            valueString << roundToInt(abs(value)*100) << " % L";
        }
        else
        {
            valueString << roundToInt((value)*100) << " % R";
        }
        
        return valueString.str();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanSlider)
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
    void setHorizontal(int numRows=1){isHorizontal = true;};
    void setVertical(int numCols=1){isHorizontal = false;};
    

private:
    
    bool isHorizontal = true;
    const std::vector<float> *source = nullptr;
    SpinLock *lock = nullptr;
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
    
    static Colour thToColour(float th, bool active);
    
private:
    
    bool isHorizontal;
    const std::vector<float> *source = nullptr;
    int ch = 0;
    SpinLock *lock = nullptr;
    
    std::vector<float> th;
    std::vector<std::unique_ptr<RoundLed>> leds;
    
    
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleChannelLedBar)
};
