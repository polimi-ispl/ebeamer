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
    steeringDirectionLeftSlider.setRange(-1,1,0.01);
    steeringDirectionLeftSlider.setValue(processor.steeringDirections[0]);
    steeringDirectionLeftSlider.addListener(this);
    steeringDirectionLeftSlider.setSliderStyle(Slider::LinearHorizontal);
    steeringDirectionLeftSlider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(steeringDirectionLeftSlider);
    
    steeringDirectionLeftLabel.setText ("Left", dontSendNotification);
    steeringDirectionLeftLabel.attachToComponent (&steeringDirectionLeftSlider, true);
    addAndMakeVisible(steeringDirectionLeftLabel);
    
    steeringDirectionRightSlider.setRange(-1,1,0.01);
    steeringDirectionRightSlider.setValue(processor.steeringDirections[0]);
    steeringDirectionRightSlider.addListener(this);
    steeringDirectionRightSlider.setSliderStyle(Slider::LinearHorizontal);
    steeringDirectionRightSlider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(steeringDirectionRightSlider);
    
    steeringDirectionRightLabel.setText ("Right", dontSendNotification);
    steeringDirectionRightLabel.attachToComponent (&steeringDirectionRightSlider, true);
    addAndMakeVisible(steeringDirectionRightLabel);
    
    
    
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
    
    passThroughButton.setBounds(20, 50, 130, 20);
    passThroughButton.setToggleState(processor.passThrough,NotificationType::dontSendNotification);
    
    bypassButton.setBounds(150, 50, 130, 20);
    bypassButton.setToggleState(processor.bypass,NotificationType::dontSendNotification);
    
    algorithmDASidealButton.setBounds(20, 100, 130, 20);
    algorithmDASidealButton.setToggleState(processor.algorithm == JucebeamAudioProcessor::DAS_IDEAL,NotificationType::dontSendNotification);
    
    algorithmDASmeasuredButton.setBounds(150, 100, 130, 20);
    algorithmDASmeasuredButton.setToggleState(processor.algorithm == JucebeamAudioProcessor::DAS_MEASURED,NotificationType::dontSendNotification);
    
    steeringDirectionLeftSlider.setBounds(50, 150, 320, 20);
    steeringDirectionLeftSlider.setValue(processor.steeringDirections[0]);
    
    steeringDirectionRightSlider.setBounds(50, 200, 320, 20);
    steeringDirectionRightSlider.setValue(processor.steeringDirections[1]);
    
    //volumeSlider.setBounds(50, 250, 150, 40);
    //volumeSlider.setValue(processor.volume);
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
    if (slider == &steeringDirectionLeftSlider)
    {
        processor.steeringDirections[0] = slider->getValue();
    }
    else if (slider == &steeringDirectionRightSlider)
    {
        processor.steeringDirections[1] = slider->getValue();
    }
}

void JucebeamAudioProcessorEditor::updateToggleState(ToggleButton* button)
{
    if (button == &algorithmDASidealButton)
    {
        if (button->getToggleState() == true)
        {
            processor.algorithm = JucebeamAudioProcessor::DAS_IDEAL;
        }
    }
    else if (button == &algorithmDASmeasuredButton)
    {
        if (button->getToggleState() == true)
        {
            processor.algorithm = JucebeamAudioProcessor::DAS_MEASURED;
        }
    }
}
