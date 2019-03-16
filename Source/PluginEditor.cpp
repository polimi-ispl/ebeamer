#include "PluginProcessor.h"
#include "PluginEditor.h"

JucebeamAudioProcessorEditor::JucebeamAudioProcessorEditor (JucebeamAudioProcessor& p)
:  AudioProcessorEditor (&p), inputMeter(p.getTotalNumInputChannels()), processor (p)
{
    DOAt = std::make_unique<DOAthread>(p);

    startTimer(EDITOR_TIMER_DURATION);

    setSize (GUI_WIDTH, GUI_HEIGHT);

    // processor.addChangeListener (this);
    addAndMakeVisible (scene);

    NormalisableRange<float> parameterRange;
    // Steering direction slider
    parameterRange = processor.steeringBeam[0]->getNormalisableRange();
    steeringBeam1Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    scene.beams[0].move(processor.steeringBeam[0]->get());
    steeringBeam1Slider.addListener(this);
    steeringBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam1Slider.setTextBoxStyle(Slider::TextBoxRight,false,60,20);
    steeringBeam1Slider.setValue(processor.steeringBeam[0]->get());
    addAndMakeVisible(steeringBeam1Slider);

    parameterRange = processor.steeringBeam[1]->getNormalisableRange();
    steeringBeam2Slider.setRange(parameterRange.start,parameterRange.end,0.01);
    steeringBeam2Slider.setValue(processor.steeringBeam[1]->get());
    scene.beams[1].move(processor.steeringBeam[1]->get());
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
    scene.beams[0].scale(processor.widthBeam[0]->get());
    widthBeam1Knob.addListener(this);
    widthBeam1Knob.setSliderStyle(Slider::Rotary);
    widthBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam1Knob.setValue(processor.widthBeam[0]->get());
    addAndMakeVisible(widthBeam1Knob);

    parameterRange = processor.widthBeam[1]->getNormalisableRange();
    widthBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    scene.beams[1].scale(processor.widthBeam[1]->get());
    widthBeam2Knob.addListener(this);
    widthBeam2Knob.setSliderStyle(Slider::Rotary);
    widthBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam2Knob.setValue(processor.widthBeam[1]->get());
    addAndMakeVisible(widthBeam2Knob);

    panLabel.setText("PAN", NotificationType::dontSendNotification);
    panLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(panLabel);

    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    panBeam1Knob.addListener(this);
    panBeam1Knob.setSliderStyle(Slider::Rotary);
    panBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam1Knob.setValue(processor.panBeam[0]->get());
    addAndMakeVisible(panBeam1Knob);

    parameterRange = processor.panBeam[0]->getNormalisableRange();
    panBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.01);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    panBeam2Knob.addListener(this);
    panBeam2Knob.setSliderStyle(Slider::Rotary);
    panBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam2Knob.setValue(processor.panBeam[1]->get());
    addAndMakeVisible(panBeam2Knob);

    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    gainLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(gainLabel);

    parameterRange = processor.gainBeam[0]->getNormalisableRange();
    gainBeam1Knob.setRange(parameterRange.start,parameterRange.end,0.1);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    gainBeam1Knob.addListener(this);
    gainBeam1Knob.setSliderStyle(Slider::Rotary);
    gainBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    gainBeam1Knob.setValue(processor.gainBeam[0]->get());
    addAndMakeVisible(gainBeam1Knob);

    parameterRange = processor.gainBeam[1]->getNormalisableRange();
    gainBeam2Knob.setRange(parameterRange.start,parameterRange.end,0.1);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    gainBeam2Knob.addListener(this);
    gainBeam2Knob.setSliderStyle(Slider::Rotary);
    gainBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    gainBeam2Knob.setValue(processor.gainBeam[1]->get());
    addAndMakeVisible(gainBeam2Knob);

    muteLabel.setText("MUTE", NotificationType::dontSendNotification);
    muteLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(muteLabel);

    beam1MuteButton.addListener(this);
    beam1MuteButton.setButtonText("1");
    setMuteButtonColor(0);
    addAndMakeVisible(beam1MuteButton);

    beam2MuteButton.addListener(this);
    beam2MuteButton.setButtonText("2");
    setMuteButtonColor(1);
    addAndMakeVisible(beam2MuteButton);

    inputMeter.setSource(p.inputRMS);
    inputMeter.startTimerHz(INPUT_RMS_UPDATE_FREQ);
    addAndMakeVisible(inputMeter);

}

JucebeamAudioProcessorEditor::~JucebeamAudioProcessorEditor()
{
    DOAt->signalThreadShouldExit();
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

    auto area = getLocalBounds();
    area.removeFromLeft(LEFT_RIGHT_MARGIN);
    area.removeFromRight(LEFT_RIGHT_MARGIN);
    area.removeFromTop(TOP_BOTTOM_MARGIN);
    area.removeFromBottom(TOP_BOTTOM_MARGIN);

    auto sceneArea = area.removeFromTop(SCENE_HEIGHT);
    sceneArea.removeFromRight((area.getWidth()-SCENE_WIDTH)/2);
    sceneArea.removeFromLeft((area.getWidth()-SCENE_WIDTH)/2);
    scene.setBounds(sceneArea);

    area.removeFromTop(STEER_SLIDER_TOP_MARGIN);
    steeringBeam1Slider.setBounds( area.removeFromTop(STEER_SLIDER_HEIGHT));
    steeringBeam2Slider.setBounds( area.removeFromTop(STEER_SLIDER_HEIGHT));

    steerLabel.setBounds(area.removeFromTop(LABEL_HEIGHT));

    area.removeFromLeft(KNOBS_LEFT_RIGHT_MARGIN);
    area.removeFromRight(KNOBS_LEFT_RIGHT_MARGIN);

    auto knobsArea = area.removeFromTop(KNOB_HEIGHT+KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    widthBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    widthBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    widthLabel.setBounds(knobsArea);

    knobsArea = area.removeFromTop(KNOB_HEIGHT+KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    panBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    panBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    panLabel.setBounds(knobsArea);

    knobsArea = area.removeFromTop(KNOB_HEIGHT+KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    gainBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    gainBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    gainLabel.setBounds(knobsArea);

    auto mutesArea = area.removeFromTop(MUTE_HEIGHT+MUTE_TOP_MARGIN);
    mutesArea.removeFromTop(MUTE_TOP_MARGIN);
    mutesArea.removeFromLeft(MUTE_LEFT_RIGHT_MARGIN);
    mutesArea.removeFromRight(MUTE_LEFT_RIGHT_MARGIN);
    beam1MuteButton.setBounds(mutesArea.removeFromLeft(MUTE_WIDTH));
    beam2MuteButton.setBounds(mutesArea.removeFromRight(MUTE_WIDTH));
    muteLabel.setBounds(mutesArea);

    auto inputLedArea = area.removeFromTop(INPUT_LED_TOP_MARGIN+INPUT_LED_HEIGHT);
    inputLedArea.removeFromTop(INPUT_LED_TOP_MARGIN);
    inputLedArea.removeFromLeft(INPUT_LEFT_RIGHT_MARGIN);
    inputLedArea.removeFromRight(INPUT_LEFT_RIGHT_MARGIN);
    inputMeter.setBounds(inputLedArea);

}

void JucebeamAudioProcessorEditor::setMuteButtonColor(uint8 beamIdx) {
    
    TextButton* beamMuteButton = beamIdx == 0 ? &beam1MuteButton : &beam2MuteButton;
    Colour colour = processor.muteBeam[beamIdx]->get() == 0 ? Colours::darkslategrey : Colours::darkred;
    beamMuteButton->setColour (TextButton::buttonColourId, colour);

}

void JucebeamAudioProcessorEditor::buttonClicked(Button *button)
{

    if(button == &beam1MuteButton)
    {
        *(processor.muteBeam[0]) = 1 - *(processor.muteBeam[0]);
        setMuteButtonColor(0);
    }
    else if(button == &beam2MuteButton)
    {
        *(processor.muteBeam[1]) = 1 - *(processor.muteBeam[1]);
        setMuteButtonColor(1);
    }
}

void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
{
    if (slider == &steeringBeam1Slider)
    {
        *(processor.steeringBeam[0]) = slider->getValue();
        scene.beams[0].move(slider->getValue());
    }
    else if (slider == &steeringBeam2Slider)
    {
        *(processor.steeringBeam[1]) = slider->getValue();
        scene.beams[1].move(slider->getValue());
    }
    else if (slider == &widthBeam1Knob)
    {
        *(processor.widthBeam[0]) = slider->getValue();
        scene.beams[0].scale(slider->getValue());
    }
    else if (slider == &widthBeam2Knob)
    {
        *(processor.widthBeam[1]) = slider->getValue();
        scene.beams[1].scale(slider->getValue());
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

void JucebeamAudioProcessorEditor::hiResTimerCallback()
{
    scene.grid.updateEnergy(DOAt->getEnergy());
}
