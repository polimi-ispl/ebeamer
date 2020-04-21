#include "PluginProcessor.h"
#include "PluginEditor.h"

JucebeamAudioProcessorEditor::JucebeamAudioProcessorEditor (EbeamerAudioProcessor& p,AudioProcessorValueTreeState& v)
:  AudioProcessorEditor (&p),  processor (p), valueTreeState(v), scene(p), cpuLoad(p)
{

    //==============================================================================
    setSize (GUI_WIDTH, GUI_HEIGHT);

    //==============================================================================
    scene.setBeamColors(beamColours);
    addAndMakeVisible (scene);

    //==============================================================================
    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    steerLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(steerLabel);
    
    steeringBeam1Label.setText("1", NotificationType::dontSendNotification);
    steeringBeam1Label.setJustificationType(Justification::left);
    steeringBeam1Label.attachToComponent(&steeringBeam1Slider, true);
    addAndMakeVisible(steeringBeam1Label);
    
    steeringBeam2Label.setText("2", NotificationType::dontSendNotification);
    steeringBeam2Label.setJustificationType(Justification::left);
    steeringBeam2Label.attachToComponent(&steeringBeam2Slider, true);
    addAndMakeVisible(steeringBeam2Label);
    
    steeringBeam1Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam1Slider.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    steeringBeam1Slider.setColour(Slider::thumbColourId, beamColours[0]);
    addAndMakeVisible(steeringBeam1Slider);

    steeringBeam2Slider.setSliderStyle(Slider::LinearHorizontal);
    steeringBeam2Slider.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    steeringBeam2Slider.setColour(Slider::thumbColourId, beamColours[1]);
    addAndMakeVisible(steeringBeam2Slider);

    steeringBeam1SliderAttachment.reset(new SliderAttachment (valueTreeState, "steerBeam1", steeringBeam1Slider));
    steeringBeam2SliderAttachment.reset(new SliderAttachment (valueTreeState, "steerBeam2", steeringBeam2Slider));

    //==============================================================================
    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    widthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(widthLabel);

    widthBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    widthBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    addAndMakeVisible(widthBeam1Knob);

    widthBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    widthBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    widthBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    addAndMakeVisible(widthBeam2Knob);
    
    widthBeam1KnobAttachment.reset(new SliderAttachment (valueTreeState, "widthBeam1", widthBeam1Knob));
    widthBeam2KnobAttachment.reset(new SliderAttachment (valueTreeState, "widthBeam2", widthBeam2Knob));

    //==============================================================================
    panLabel.setText("PAN", NotificationType::dontSendNotification);
    panLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(panLabel);

    panBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    panBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    addAndMakeVisible(panBeam1Knob);

    panBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    panBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    panBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    addAndMakeVisible(panBeam2Knob);
    
    panBeam1KnobAttachment.reset(new SliderAttachment (valueTreeState, "panBeam1", panBeam1Knob));
    panBeam2KnobAttachment.reset(new SliderAttachment (valueTreeState, "panBeam2", panBeam2Knob));

    //==============================================================================

    levelLabel.setText("LEVEL", NotificationType::dontSendNotification);
    levelLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(levelLabel);

    levelBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    levelBeam1Knob.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    levelBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    addAndMakeVisible(levelBeam1Knob);

    levelBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    levelBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft,false,LABEL_WIDTH,LABEL_HEIGHT);
    levelBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    addAndMakeVisible(levelBeam2Knob);

    levelBeam1KnobAttachment.reset(new SliderAttachment (valueTreeState, "levelBeam1", levelBeam1Knob));
    levelBeam2KnobAttachment.reset(new SliderAttachment (valueTreeState, "levelBeam2", levelBeam2Knob));

    //==============================================================================
    
    muteLabel.setText("MUTE", NotificationType::dontSendNotification);
    muteLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(muteLabel);

    muteBeam1Button.setButtonText("1");
    addAndMakeVisible(muteBeam1Button);

    muteBeam2Button.setButtonText("2");
    addAndMakeVisible(muteBeam2Button);
    
    beam1MuteButtonAttachment.reset(new ButtonAttachment (valueTreeState, "muteBeam1", muteBeam1Button));
    beam2MuteButtonAttachment.reset(new ButtonAttachment (valueTreeState, "muteBeam2", muteBeam2Button));
    
    getLookAndFeel().setColour (MuteButton::buttonOnColourId, Colours::darkred);

    //==============================================================================
    
    beam1Meter.setSource(p.beamMeters,0,p.beamMetersLock);
    beam1Meter.startTimerHz(BEAM_METER_UPDATE_FREQ);
    addAndMakeVisible(beam1Meter);
    
    beam2Meter.setSource(p.beamMeters,1,p.beamMetersLock);
    beam2Meter.startTimerHz(BEAM_METER_UPDATE_FREQ);
    addAndMakeVisible(beam2Meter);

    //==============================================================================

    hpfLabel.setText("HPF", NotificationType::dontSendNotification);
    hpfLabel.setJustificationType(Justification::left);
    hpfLabel.attachToComponent(&hpfSlider, true);
    
    hpfSlider.setSliderStyle(Slider::LinearHorizontal);
    hpfSlider.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    addAndMakeVisible(hpfSlider);
    
    hpfSliderAttachment.reset(new SliderAttachment (valueTreeState, "hpf", hpfSlider));

    //==============================================================================
    
    inputMeter.setSource(processor.inputMeters, processor.inputMetersLock);
    inputMeter.startTimerHz(INPUT_METER_UPDATE_FREQ);
    addAndMakeVisible(inputMeter);
    
    //==============================================================================
    
    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    gainLabel.setJustificationType(Justification::left);
    gainLabel.attachToComponent(&gainSlider, true);
    
    gainSlider.setSliderStyle(Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(Slider::TextBoxRight,false,LABEL_WIDTH,LABEL_HEIGHT);
    addAndMakeVisible(gainSlider);
    
    gainSliderAttachment.reset(new SliderAttachment (valueTreeState, "gainMic", gainSlider));
    
    //=====================================================
    // Add CPU Load and start its timer
    cpuLoad.startTimerHz(CPULOAD_UPDATE_FREQ);
    addAndMakeVisible(cpuLoad);
    
    //=====================================================
    // Add front facing toggle
    frontToggleLabel.setText("FLIP", NotificationType::dontSendNotification);
    frontToggleLabel.setFont(10);
    frontToggleLabel.attachToComponent(&frontToggle, true);
    addAndMakeVisible(frontToggle);
    
    frontToggleAttachment.reset(new ButtonAttachment (valueTreeState, "frontFacing", frontToggle));
    
    //=====================================================
    // Configuration selection combo
    configComboLabel.setText("CONFIG", NotificationType::dontSendNotification);
    configComboLabel.setFont(10);
    configComboLabel.attachToComponent(&configCombo, true);
    configCombo.addItemList(micConfigLabels, 10);
    addAndMakeVisible(configCombo);
    
    configComboLabelAttachment.reset(new ComboBoxAttachment (valueTreeState, "config", configCombo));
    
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
    steeringBeam1Slider.setBounds( area.removeFromTop(STEER_SLIDER_HEIGHT).withTrimmedLeft(LABEL_BEAM_WIDTH));
    steeringBeam2Slider.setBounds( area.removeFromTop(STEER_SLIDER_HEIGHT).withTrimmedLeft(LABEL_BEAM_WIDTH));

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
    levelBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    levelBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    auto meterArea = knobsArea.removeFromLeft(BEAM_LED_WIDTH+BEAM_LEFT_RIGHT_MARGIN);
    meterArea.removeFromTop(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromBottom(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromLeft(BEAM_LEFT_RIGHT_MARGIN);
    beam1Meter.setBounds(meterArea.removeFromLeft(BEAM_LED_WIDTH));
    meterArea = knobsArea.removeFromRight(BEAM_LED_WIDTH+BEAM_LEFT_RIGHT_MARGIN);
    meterArea.removeFromTop(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromBottom(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromRight(BEAM_LEFT_RIGHT_MARGIN);
    beam2Meter.setBounds(meterArea.removeFromRight(BEAM_LED_WIDTH));
    levelLabel.setBounds(knobsArea);

    auto mutesArea = area.removeFromTop(MUTE_HEIGHT+MUTE_TOP_MARGIN);
    mutesArea.removeFromTop(MUTE_TOP_MARGIN);
    mutesArea.removeFromLeft(MUTE_LEFT_RIGHT_MARGIN);
    mutesArea.removeFromRight(MUTE_LEFT_RIGHT_MARGIN);
    muteBeam1Button.setBounds(mutesArea.removeFromLeft(MUTE_WIDTH));
    muteBeam2Button.setBounds(mutesArea.removeFromRight(MUTE_WIDTH));
    muteLabel.setBounds(mutesArea);

    area.removeFromTop(INPUT_SECTION_TOP_MARGIN);
    hpfSlider.setBounds(area.removeFromTop(INPUT_HPF_SLIDER_HEIGHT).withTrimmedLeft(INPUT_HPF_LABEL_WIDTH));
    
    auto inputLedArea = area.removeFromTop(INPUT_LED_HEIGHT);
    inputLedArea.removeFromLeft(INPUT_LEFT_RIGHT_MARGIN);
    inputLedArea.removeFromRight(INPUT_LEFT_RIGHT_MARGIN);
    inputMeter.setBounds(inputLedArea);
    
    gainSlider.setBounds(area.removeFromTop(INPUT_GAIN_SLIDER_HEIGHT).withTrimmedLeft(INPUT_GAIN_LABEL_WIDTH));
    
    //===============================================================
    /** Prepare area for the performance monitor */
    auto performanceMonitorArea = area.removeFromTop(PREFORMANCE_MONITOR_HEIGHT);
    
    /** Set area for CPU Load */
    cpuLoad.setBounds(performanceMonitorArea.removeFromLeft(CPULOAD_WIDTH));

    /** Set area for front toggle */
    performanceMonitorArea.removeFromLeft(FRONT_TOGGLE_LABEL_WIDTH);
    frontToggle.setBounds(performanceMonitorArea.removeFromLeft(FRONT_TOGGLE_WIDTH));
    
    /** Set area for config combo */
    performanceMonitorArea.removeFromLeft(CONFIG_COMBO_LABEL_WIDTH);
    configCombo.setBounds(performanceMonitorArea.removeFromLeft(CONFIG_COMBO_WIDTH));
}

//void JucebeamAudioProcessorEditor::setMuteButtonColor(uint8 beamIdx) {
//
//    TextButton* beamMuteButton = beamIdx == 0 ? &muteBeam1Button : &muteBeam2Button;
//    Colour colour = (bool)processor.muteBeamParam[beamIdx].getValue() == 0 ? Colours::darkslategrey : Colours::darkred;
//    beamMuteButton->setColour (TextButton::buttonColourId, colour);
//
//}
//
//void JucebeamAudioProcessorEditor::buttonClicked(Button *button){
//    if(button == &muteBeam1Button){
//        const bool val = processor.muteBeamParam[0].getValue();
//        printf("%d\n",val);
////        processor.muteBeamParam[0].setValue(~val);
////        scene.beams[0].setStatus((bool)processor.muteBeamParam[0].getValue()==0);
//    }
//    else if(button == &muteBeam2Button){
//        processor.muteBeamParam[1].setValue(!(bool)processor.muteBeamParam[1].getValue());
////        scene.beams[1].setStatus((bool)processor.muteBeamParam[1].getValue()==0);
//    }
//}
//
//void JucebeamAudioProcessorEditor::buttonStateChanged(Button *button){
//    if (button == &frontToggle){
//        (processor.frontFacingParam.getValue()) = frontToggle.getToggleState();
//        scene.repaint();
//    }
//}
//
//void JucebeamAudioProcessorEditor::sliderValueChanged(Slider *slider)
//{
////    if (slider == &steeringBeam1Slider)
////    {
////        *(processor.steeringBeamParam[0]) = slider->getValue();
////        scene.beams[0].move(slider->getValue());
////    }
////    else if (slider == &steeringBeam2Slider)
////    {
////        *(processor.steeringBeamParam[1]) = slider->getValue();
////        scene.beams[1].move(slider->getValue());
////    }
////    else if (slider == &widthBeam1Knob)
////    {
////        *(processor.widthBeamParam[0]) = slider->getValue();
////        scene.beams[0].scale(slider->getValue());
////    }
////    else if (slider == &widthBeam2Knob)
////    {
////        *(processor.widthBeamParam[1]) = slider->getValue();
////        scene.beams[1].scale(slider->getValue());
////    }
////    else if (slider == &panBeam1Knob)
////    {
////        *(processor.panBeamParam[0]) = slider->getValue();
////    }
////    else if (slider == &panBeam2Knob)
////    {
////        *(processor.panBeamParam[1]) = slider->getValue();
////    }
////    else if (slider == &levelBeam1Knob)
////    {
////        *(processor.levelBeamParam[0]) = slider->getValue();
////    }
////    else if (slider == &levelBeam2Knob)
////    {
////        *(processor.levelBeamParam[1]) = slider->getValue();
////    }
////    else if(slider == &hpfSlider)
////    {
////        *(processor.hpfFreqParam) = slider->getValue();
////    }
////    else if(slider == &gainSlider)
////    {
////        *(processor.micGainParam) = slider->getValue();
////    }
//}
//
//void JucebeamAudioProcessorEditor::comboBoxChanged(ComboBox *combo){
//    if (combo == &configCombo){
////        (*processor.configParam) = configCombo.getSelectedItemIndex();
//        processor.setMicConfig(static_cast<MicConfig>(configCombo.getSelectedItemIndex()));
//    }
//}
