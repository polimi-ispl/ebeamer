/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JucebeamAudioProcessorEditor::JucebeamAudioProcessorEditor (JucebeamAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    // Hp Enable button
    passThroughButton.setButtonText("Pass-through");
    passThroughButton.addListener(this);
    addAndMakeVisible(passThroughButton);
    
    // Hp Enable button
    bypassButton.setButtonText("Bypass");
    bypassButton.addListener(this);
    addAndMakeVisible(bypassButton);
    
    // algorithms buttons
    algorithmDASidealButton.setButtonText("DAS ideal");
    algorithmDASidealButton.onClick = [this] { updateToggleState (&algorithmDASidealButton); };
    algorithmDASidealButton.setRadioGroupId(RadioButtonIds::AlgorithmType);
    addAndMakeVisible(algorithmDASidealButton);
    
    algorithmDASmeasuredButton.setButtonText("DAS measured");
    algorithmDASmeasuredButton.onClick = [this] { updateToggleState (&algorithmDASmeasuredButton); };
    algorithmDASmeasuredButton.setRadioGroupId(RadioButtonIds::AlgorithmType);
    addAndMakeVisible(algorithmDASmeasuredButton);
    
    // Steering direction slider
    steeringDirectionSlider.setRange(0, 23, 1);
    steeringDirectionSlider.addListener(this);
    steeringDirectionSlider.setSliderStyle(Slider::LinearHorizontal);
    steeringDirectionSlider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(steeringDirectionSlider);
    
}

JucebeamAudioProcessorEditor::~JucebeamAudioProcessorEditor()
{
}

//==============================================================================
void JucebeamAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("eStick Beamer", 100, 0, 200, 40, Justification::centred, 1);
}

void JucebeamAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    passThroughButton.setBounds(20, 40, 130, 20);
    passThroughButton.setToggleState(processor.passThrough,NotificationType::dontSendNotification);
    
    bypassButton.setBounds(150, 40, 130, 20);
    bypassButton.setToggleState(processor.bypass,NotificationType::dontSendNotification);
    
    algorithmDASidealButton.setBounds(20, 80, 130, 20);
    algorithmDASidealButton.setToggleState(processor.algorithm == JucebeamAudioProcessor::DAS_IDEAL,NotificationType::dontSendNotification);
    
    algorithmDASmeasuredButton.setBounds(150, 80, 130, 20);
    algorithmDASmeasuredButton.setToggleState(processor.algorithm == JucebeamAudioProcessor::DAS_MEASURED,NotificationType::dontSendNotification);
    
    steeringDirectionSlider.setBounds(20, 120, 360, 20);
    steeringDirectionSlider.setValue(processor.steeringDirection);
}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{
    if (button == &passThroughButton)
    {
        processor.passThrough = passThroughButton.getToggleState();
    }
    else if (button == &bypassButton)
    {
        processor.bypass = bypassButton.getToggleState();
    }
}

void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &steeringDirectionSlider)
    {
        processor.steeringDirection = slider->getValue();
    }
}

void JucebeamAudioProcessorEditor::updateToggleState(ToggleButton* button)
{
    if (button == &algorithmDASmeasuredButton)
    {
        if (button->getToggleState() == true)
        {
            processor.algorithm = JucebeamAudioProcessor::DAS_MEASURED;
        }
    }
    else if (button == &algorithmDASidealButton)
    {
        if (button->getToggleState() == true)
        {
            processor.algorithm = JucebeamAudioProcessor::DAS_MEASURED;
        }
    }
}
