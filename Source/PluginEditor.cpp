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
    
    NormalisableRange<float> parameterRange;
    
    //TODO: Soundfield display
    
    // Steering direction slider
    parameterRange = processor.steeringBeam[0]->getNormalisableRange();
    steeringBeam1Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    steeringBeam1Slider.addListener(this);
    steeringBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam1Slider.setTextBoxStyle(Slider::TextBoxRight,false,60,20);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    addAndMakeVisible(steeringBeam1Slider);
    
    parameterRange = processor.steeringBeam[1]->getNormalisableRange();
    steeringBeam2Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    steeringBeam2Slider.addListener(this);
    steeringBeam2Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam2Slider.setTextBoxStyle(Slider::TextBoxRight,false,60,20);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    addAndMakeVisible(steeringBeam2Slider);
    
    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    steerLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(steerLabel);
    
    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    widthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(widthLabel);
    
    parameterRange = processor.widthBeam[0]->getNormalisableRange();
    widthBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    widthBeam1Knob.addListener(this);
    widthBeam1Knob.setSliderStyle(Slider::Rotary);
    widthBeam1Knob.setTextBoxStyle(Slider::TextBoxLeft,false,labelWidth,20);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    addAndMakeVisible(widthBeam1Knob);
    
    parameterRange = processor.widthBeam[1]->getNormalisableRange();
    widthBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    widthBeam2Knob.addListener(this);
    widthBeam2Knob.setSliderStyle(Slider::Rotary);
    widthBeam2Knob.setTextBoxStyle(Slider::TextBoxRight,false,labelWidth,20);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    addAndMakeVisible(widthBeam2Knob);
    
    panLabel.setText("PAN", NotificationType::dontSendNotification);
    addAndMakeVisible(panLabel);
    
    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    panBeam1Knob.addListener(this);
    panBeam1Knob.setSliderStyle(Slider::Rotary);
    panBeam1Knob.setTextBoxStyle(Slider::TextBoxLeft,false,labelWidth,20);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    addAndMakeVisible(panBeam1Knob);
    
    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    panBeam2Knob.addListener(this);
    panBeam2Knob.setSliderStyle(Slider::Rotary);
    panBeam2Knob.setTextBoxStyle(Slider::TextBoxRight,false,labelWidth,20);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    addAndMakeVisible(panBeam2Knob);
    
    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    addAndMakeVisible(gainLabel);
    
    parameterRange = processor.gainBeam[0]->getNormalisableRange();
    gainBeam1Knob.setRange(parameterRange.start,parameterRange.end,1);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    gainBeam1Knob.addListener(this);
    gainBeam1Knob.setSliderStyle(Slider::Rotary);
    gainBeam1Knob.setTextBoxStyle(Slider::TextBoxLeft,false,labelWidth,20);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    addAndMakeVisible(gainBeam1Knob);
    
    parameterRange = processor.gainBeam[1]->getNormalisableRange();
    gainBeam2Knob.setRange(parameterRange.start,parameterRange.end,1);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    gainBeam2Knob.addListener(this);
    gainBeam2Knob.setSliderStyle(Slider::Rotary);
    gainBeam2Knob.setTextBoxStyle(Slider::TextBoxRight,false,labelWidth,20);
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
    
    steeringBeam1Slider.setBounds(75, 275, 450, 20);
    steeringBeam2Slider.setBounds(75, 275+30, 450, 20);
    
    steerLabel.setBounds(300-25, 340-10, 50, 20);
    
    widthLabel.setBounds(300-25, 400-10, 50, 20);
    widthBeam1Knob.setBounds(160-(knobSize>>1)-labelWidth, 400-(knobSize>>1), knobSize+labelWidth, knobSize);
    widthBeam2Knob.setBounds(440-(knobSize>>1), 400-(knobSize>>1), knobSize+labelWidth, knobSize);
    
    panLabel.setBounds(300-25, 500-10, 50, 20);
    panBeam1Knob.setBounds(160-(knobSize>>1)-labelWidth, 500-(knobSize>>1), knobSize+labelWidth, knobSize);
    panBeam2Knob.setBounds(440-(knobSize>>1), 500-(knobSize>>1), knobSize+labelWidth, knobSize);
    
    gainLabel.setBounds(300-25, 600-10, 50, 20);
    gainBeam1Knob.setBounds(160-(knobSize>>1)-labelWidth, 600-(knobSize>>1), knobSize+labelWidth, knobSize);
    gainBeam2Knob.setBounds(440-(knobSize>>1), 600-(knobSize>>1), knobSize+labelWidth, knobSize);
    
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


