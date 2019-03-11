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
    steeringBeam1Slider.setRange(-1,1,0.01);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    steeringBeam1Slider.addListener(this);
    steeringBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam1Slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    addAndMakeVisible(steeringBeam1Slider);
    
    steeringBeam2Slider.setRange(-1,1,0.01);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    steeringBeam2Slider.addListener(this);
    steeringBeam2Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam2Slider.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    addAndMakeVisible(steeringBeam2Slider);
    
    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    steerLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(steerLabel);
    
    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    widthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(widthLabel);
    
    widthBeam1Knob.setRange(0,1,0.01);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    widthBeam1Knob.addListener(this);
    widthBeam1Knob.setSliderStyle(Slider::Rotary);
    widthBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    addAndMakeVisible(widthBeam1Knob);
    
    widthBeam2Knob.setRange(0,1,0.01);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    widthBeam2Knob.addListener(this);
    widthBeam2Knob.setSliderStyle(Slider::Rotary);
    widthBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    addAndMakeVisible(widthBeam2Knob);
    
    panLabel.setText("PAN", NotificationType::dontSendNotification);
    addAndMakeVisible(panLabel);
    
    panBeam1Knob.setRange(-1,1,0.01);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    panBeam1Knob.addListener(this);
    panBeam1Knob.setSliderStyle(Slider::Rotary);
    panBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    addAndMakeVisible(panBeam1Knob);
    
    panBeam2Knob.setRange(-1,1,0.01);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    panBeam2Knob.addListener(this);
    panBeam2Knob.setSliderStyle(Slider::Rotary);
    panBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    addAndMakeVisible(panBeam2Knob);
    
    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    addAndMakeVisible(gainLabel);
    
    //TODO decibel gain
    gainBeam1Knob.setRange(1,100,1);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    gainBeam1Knob.addListener(this);
    gainBeam1Knob.setSliderStyle(Slider::Rotary);
    gainBeam1Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    addAndMakeVisible(gainBeam1Knob);
    
    gainBeam2Knob.setRange(1,100,1);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    gainBeam2Knob.addListener(this);
    gainBeam2Knob.setSliderStyle(Slider::Rotary);
    gainBeam2Knob.setTextBoxStyle(Slider::NoTextBox,false,0,0);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    addAndMakeVisible(gainBeam2Knob);
    
    muteLabel.setText("MUTE", NotificationType::dontSendNotification);
    addAndMakeVisible(muteLabel);
    
    beam1MuteButton.addListener(this);
    beam1MuteButton.setButtonText("1");
    addAndMakeVisible(beam1MuteButton);
    
    beam2MuteButton.addListener(this);
    beam2MuteButton.setButtonText("2");
    addAndMakeVisible(beam2MuteButton);

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
    
    uint8 knobSize = 90;
    uint8 muteSize = 40;
    
    steeringBeam1Slider.setBounds(75, 275, 450, 20);
    steeringBeam2Slider.setBounds(75, 275+30, 450, 20);
    
    steerLabel.setBounds(300-25, 340-10, 50, 20);
    
    widthLabel.setBounds(300-25, 400-10, 50, 20);
    widthBeam1Knob.setBounds(160-(knobSize>>1), 400-(knobSize>>1), knobSize, knobSize);
    widthBeam2Knob.setBounds(440-(knobSize>>1), 400-(knobSize>>1), knobSize, knobSize);
    
    panLabel.setBounds(300-25, 500-10, 50, 20);
    panBeam1Knob.setBounds(160-(knobSize>>1), 500-(knobSize>>1), knobSize, knobSize);
    panBeam2Knob.setBounds(440-(knobSize>>1), 500-(knobSize>>1), knobSize, knobSize);
    
    gainLabel.setBounds(300-25, 600-10, 50, 20);
    gainBeam1Knob.setBounds(160-(knobSize>>1), 600-(knobSize>>1), knobSize, knobSize);
    gainBeam2Knob.setBounds(440-(knobSize>>1), 600-(knobSize>>1), knobSize, knobSize);
    
    muteLabel.setBounds(300-25, 670-10, 50, 20);
    beam1MuteButton.setBounds(160-(muteSize>>1), 670-(muteSize>>1), muteSize, muteSize);
    beam2MuteButton.setBounds(440-(muteSize>>1), 670-(muteSize>>1), muteSize, muteSize);
    
}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{
    if(button == &beam1MuteButton)
    {
        *(processor.muteBeam[0]) = 1 - *(processor.muteBeam[0]);
    }
    else if(button == &beam2MuteButton)
    {
        *(processor.muteBeam[1]) = 1 - *(processor.muteBeam[1]);
    }
}

void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &steeringBeam1Slider)
    {
        *(processor.steeringBeam[0]) = slider->getValue();
    }
    else if (slider == &steeringBeam2Slider)
    {
        *(processor.steeringBeam[1]) = slider->getValue();
    }
    else if (slider == &widthBeam1Knob)
    {
        *(processor.widthBeam[0]) = slider->getValue();
    }
    else if (slider == &widthBeam2Knob)
    {
        *(processor.widthBeam[1]) = slider->getValue();
    }
    else if (slider == &panBeam1Knob)
    {
        *(processor.panBeam[0]) = slider->getValue();
    }
    else if (slider == &panBeam2Knob)
    {
        *(processor.panBeam[1]) = slider->getValue();
    }
    else if (slider == &gainBeam1Knob)
    {
        *(processor.gainBeam[0]) = slider->getValue();
    }
    else if (slider == &gainBeam2Knob)
    {
        *(processor.gainBeam[1]) = slider->getValue();
    }
}

