/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
class DecibelSlider : public Slider
{
public:
    DecibelSlider() {}
    
    double getValueFromText (const String& text) override
    {
        auto minusInfinitydB = -100.0;
        
        auto decibelText = text.upToFirstOccurrenceOf ("dB", false, false).trim();    // [1]
        
        return decibelText.equalsIgnoreCase ("-INF") ? minusInfinitydB
        : decibelText.getDoubleValue();  // [2]
    }
    
    String getTextFromValue (double value) override
    {
        return Decibels::toString (value,0);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecibelSlider)
};

//==============================================================================
/**
*/
class JucebeamAudioProcessorEditor  : public AudioProcessorEditor,
                                      private ToggleButton::Listener,
                                      private Slider::Listener
{
public:
    
    
    JucebeamAudioProcessorEditor (JucebeamAudioProcessor&);
    ~JucebeamAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JucebeamAudioProcessor& processor;
    
    // Project specific
    
    //TODO: Soundfield display
    
    Slider steeringBeam1Slider;
    Slider steeringBeam2Slider;
    
    Label steerLabel;
    
    Label widthLabel;
    Slider widthBeam1Knob;
    Slider widthBeam2Knob;
    
    Label panLabel;
    Slider panBeam1Knob;
    Slider panBeam2Knob;
    
    Label gainLabel;
    DecibelSlider gainBeam1Knob;
    DecibelSlider gainBeam2Knob;
    
    Label muteLabel;
    ToggleButton beam1MuteButton;
    ToggleButton beam2MuteButton;
    
    const uint8 knobSize = 90;
    const uint8 muteSize = 40;
    const uint8 labelWidth = 60;
    
    // TODO: Meters

    
    // Callbacks
    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
