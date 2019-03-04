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
    ToggleButton passThroughButton, bypassButton;
    void buttonClicked(Button *button) override;
    
    Label steeringDirectionLeftLabel;
    Label steeringDirectionRightLabel;
    Slider steeringDirectionLeftSlider;
    Slider steeringDirectionRightSlider;
    void sliderValueChanged(Slider *slider) override;
    
    ToggleButton algorithmDASidealButton, algorithmDASmeasuredButton;
    enum RadioButtonIds
    {
        AlgorithmType = 1001
    };
    void updateToggleState(ToggleButton* button);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
