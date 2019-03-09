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
    setSize (600, 800);
    
    //TODO: Soundfield display
    
    // Steering direction slider
    steeringDirectionBeam1Slider.setRange(-1,1,0.01);
    steeringDirectionBeam1Slider.setValue(processor.steeringDirectionBeam1->get());
    steeringDirectionBeam1Slider.addListener(this);
    steeringDirectionBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringDirectionBeam1Slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(steeringDirectionBeam1Slider);
    
    steeringDirectionBeam2Slider.setRange(-1,1,0.01);
    steeringDirectionBeam2Slider.setValue(processor.steeringDirectionBeam2->get());
    steeringDirectionBeam2Slider.addListener(this);
    steeringDirectionBeam2Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringDirectionBeam2Slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(steeringDirectionBeam2Slider);
    /*
    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    addAndMakeVisible(steerLabel);
    
    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    addAndMakeVisible(widthLabel);
    
    widthBeam1Knob.setRange(0,1,0.01);
    widthBeam1Knob.setValue(processor.widthBeam1->get());
    widthBeam1Knob.addListener(this);
    widthBeam1Knob.setSliderStyle(Slider::RotaryHorizontalDrag);
    widthBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(widthBeam1Knob);
    
    widthBeam2Knob.setRange(0,1,0.01);
    widthBeam2Knob.setValue(processor.widthBeam2->get());
    widthBeam2Knob.addListener(this);
    widthBeam2Knob.setSliderStyle(Slider::RotaryHorizontalDrag);
    widthBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(widthBeam2Knob);
    
    panLabel.setText("PAN", NotificationType::dontSendNotification);
    addAndMakeVisible(panLabel);
    
    panBeam1Knob.setRange(-1,1,0.01);
    panBeam1Knob.setValue(processor.panBeam1->get());
    panBeam1Knob.addListener(this);
    panBeam1Knob.setSliderStyle(Slider::RotaryHorizontalDrag);
    panBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(panBeam1Knob);
    
    panBeam2Knob.setRange(-1,1,0.01);
    panBeam2Knob.setValue(processor.panBeam2->get());
    panBeam2Knob.addListener(this);
    panBeam2Knob.setSliderStyle(Slider::RotaryHorizontalDrag);
    panBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(panBeam2Knob);
    
    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    addAndMakeVisible(gainLabel);
    
    gainBeam1Knob.setRange(0,1,0.01);
    gainBeam1Knob.setValue(processor.gainBeam1->get());
    gainBeam1Knob.addListener(this);
    gainBeam1Knob.setSliderStyle(Slider::RotaryVerticalDrag);
    gainBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(gainBeam1Knob);
    
    gainBeam2Knob.setRange(0,1,0.01);
    gainBeam2Knob.setValue(processor.gainBeam2->get());
    gainBeam2Knob.addListener(this);
    gainBeam2Knob.setSliderStyle(Slider::RotaryVerticalDrag);
    gainBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    addAndMakeVisible(gainBeam2Knob);
*/
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

}

void JucebeamAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    steeringDirectionBeam1Slider.setBounds(50, 150, 320, 20);
    steeringDirectionBeam1Slider.setValue(processor.steeringDirectionBeam1->get());
    
}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{
    
}

void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &steeringDirectionBeam1Slider)
    {
        *(processor.steeringDirectionBeam1) = slider->getValue();
    }
    else if (slider == &steeringDirectionBeam2Slider)
    {
        *(processor.steeringDirectionBeam2) = slider->getValue();
    }
}
