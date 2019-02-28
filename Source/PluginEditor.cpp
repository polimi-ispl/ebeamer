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
    hpEnableButton.setButtonText("Enable HP filter");
    hpEnableButton.addListener(this);
    addAndMakeVisible(hpEnableButton);
    
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
    g.drawFittedText ("Beam", 100, 10, 200, 40, Justification::left, 1);
}

void JucebeamAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    hpEnableButton.setBounds(20, 40, 150, 40);
    hpEnableButton.setToggleState(processor.hpEnable,NotificationType::dontSendNotification);
    
}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{
    processor.hpEnable = hpEnableButton.getToggleState();
}
