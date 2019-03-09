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
    
    Slider steeringDirectionBeam1Slider;
    Slider steeringDirectionBeam2Slider;
    
    Label steerLabel;
    
    Label widthLabel;
    Slider widthBeam1Knob;
    Slider widthBeam2Knob;
    
    Label panLabel;
    Slider panBeam1Knob;
    Slider panBeam2Knob;
    
    Label gainLabel;
    Slider gainBeam1Knob;
    Slider gainBeam2Knob;
    
    Label muteLabel;
    ImageButton beam1MuteButton;
    ImageButton beam2MuteButton;
    
    // TODO: Meters

    
    // Callbacks
    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
