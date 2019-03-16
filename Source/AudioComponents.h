/*
  ==============================================================================

    AudioComponents.h
    Created: 16 Mar 2019 11:44:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once
#define DB_MINUS_INF -100.0

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class DecibelSlider : public Slider
{
public:
    
    DecibelSlider(){};
    
    double getValueFromText (const String& text) override
    {
       
        auto decibelText = text.upToFirstOccurrenceOf ("dB", false, false).trim();    // [1]
        
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


class LedComponent : public Component
{
public:
    
    LedComponent(){};
    
    Colour colour;
    
    void paint(Graphics&) override;
    void resized() override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LedComponent)
};

class LedBarComponent : public Component
{
public:
    
    LedBarComponent(int num, bool isHorizontal = true);
    std::vector<std::unique_ptr<LedComponent>> leds;
    void paint(Graphics&) override;
    void resized() override;

private:
    
    bool isHorizontal;
    int num;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LedBarComponent)
};
