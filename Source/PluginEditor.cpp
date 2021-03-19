/*
 eBeamer Plugin Processor GUI
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

EBeamerAudioProcessorEditor::EBeamerAudioProcessorEditor(EbeamerAudioProcessor &p, AudioProcessorValueTreeState &v)
: AudioProcessorEditor(&p), processor(p), valueTreeState(v) {
    
    //==============================================================================
    setSize(GUI_WIDTH, GUI_HEIGHT);
    
    //==============================================================================
    scene.setCallback(&p);
    scene.setBeamColors(beamColours);
    addAndMakeVisible(scene);
    
    //==============================================================================
    steerLabel.setText("STEER", NotificationType::dontSendNotification);
    steerLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(steerLabel);
    
    steerBeam1Label.setText("1", NotificationType::dontSendNotification);
    steerBeam1Label.setJustificationType(Justification::left);
    steerBeam1Label.attachToComponent(&steerBeamX1Slider, true);
    addAndMakeVisible(steerBeam1Label);
    
    steerBeam2Label.setText("2", NotificationType::dontSendNotification);
    steerBeam2Label.setJustificationType(Justification::left);
    steerBeam2Label.attachToComponent(&steerBeamX2Slider, true);
    addAndMakeVisible(steerBeam2Label);
    
    steerBeamX1Slider.setSliderStyle(Slider::LinearHorizontal);
    steerBeamX1Slider.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    steerBeamX1Slider.setColour(Slider::thumbColourId, beamColours[0]);
    steerBeamX1Slider.setPopupMenuEnabled(true);
    steerBeamX1Slider.setCallback(&processor, "steerBeamX1");
    addAndMakeVisible(steerBeamX1Slider);
    
    steerBeamX2Slider.setSliderStyle(Slider::LinearHorizontal);
    steerBeamX2Slider.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    steerBeamX2Slider.setColour(Slider::thumbColourId, beamColours[1]);
    steerBeamX2Slider.setPopupMenuEnabled(true);
    steerBeamX2Slider.setCallback(&processor, "steerBeamX2");
    addAndMakeVisible(steerBeamX2Slider);
    
    steerBeamY1Slider.setSliderStyle(Slider::LinearVertical);
    steerBeamY1Slider.setTextBoxStyle(Slider::TextBoxAbove, false, LABEL_WIDTH, LABEL_HEIGHT);
    steerBeamY1Slider.setColour(Slider::thumbColourId, beamColours[0]);
    steerBeamY1Slider.setPopupMenuEnabled(true);
    steerBeamY1Slider.setCallback(&processor, "steerBeamY1");
    addAndMakeVisible(steerBeamY1Slider);
    
    steerBeamY2Slider.setSliderStyle(Slider::LinearVertical);
    steerBeamY2Slider.setTextBoxStyle(Slider::TextBoxAbove, false, LABEL_WIDTH, LABEL_HEIGHT);
    steerBeamY2Slider.setColour(Slider::thumbColourId, beamColours[1]);
    steerBeamY2Slider.setPopupMenuEnabled(true);
    steerBeamY2Slider.setCallback(&processor, "steerBeamY2");
    addAndMakeVisible(steerBeamY2Slider);
    
    steerBeamX1SliderAttachment.reset(new SliderAttachment(valueTreeState, "steerBeamX1", steerBeamX1Slider));
    steerBeamX2SliderAttachment.reset(new SliderAttachment(valueTreeState, "steerBeamX2", steerBeamX2Slider));
    steerBeamY1SliderAttachment.reset(new SliderAttachment(valueTreeState, "steerBeamY1", steerBeamY1Slider));
    steerBeamY2SliderAttachment.reset(new SliderAttachment(valueTreeState, "steerBeamY2", steerBeamY2Slider));
    
    //==============================================================================
    widthLabel.setText("WIDTH", NotificationType::dontSendNotification);
    widthLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(widthLabel);
    
    widthBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    widthBeam1Knob.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    widthBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    widthBeam1Knob.setPopupMenuEnabled(true);
    widthBeam1Knob.setCallback(&processor, "widthBeam1");
    addAndMakeVisible(widthBeam1Knob);
    
    widthBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    widthBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft, false, LABEL_WIDTH, LABEL_HEIGHT);
    widthBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    widthBeam2Knob.setPopupMenuEnabled(true);
    widthBeam2Knob.setCallback(&processor, "widthBeam2");
    addAndMakeVisible(widthBeam2Knob);
    
    widthBeam1KnobAttachment.reset(new SliderAttachment(valueTreeState, "widthBeam1", widthBeam1Knob));
    widthBeam2KnobAttachment.reset(new SliderAttachment(valueTreeState, "widthBeam2", widthBeam2Knob));
    
    //==============================================================================
    panLabel.setText("PAN", NotificationType::dontSendNotification);
    panLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(panLabel);
    
    panBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    panBeam1Knob.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    panBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    panBeam1Knob.setPopupMenuEnabled(true);
    panBeam1Knob.setCallback(&processor, "panBeam1");
    addAndMakeVisible(panBeam1Knob);
    
    panBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    panBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft, false, LABEL_WIDTH, LABEL_HEIGHT);
    panBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    panBeam2Knob.setPopupMenuEnabled(true);
    panBeam2Knob.setCallback(&processor, "panBeam2");
    addAndMakeVisible(panBeam2Knob);
    
    panBeam1KnobAttachment.reset(new SliderAttachment(valueTreeState, "panBeam1", panBeam1Knob));
    panBeam2KnobAttachment.reset(new SliderAttachment(valueTreeState, "panBeam2", panBeam2Knob));
    
    //==============================================================================
    
    levelLabel.setText("LEVEL", NotificationType::dontSendNotification);
    levelLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(levelLabel);
    
    levelBeam1Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    levelBeam1Knob.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    levelBeam1Knob.setColour(Slider::thumbColourId, beamColours[0]);
    levelBeam1Knob.setPopupMenuEnabled(true);
    levelBeam1Knob.setCallback(&processor, "levelBeam1");
    addAndMakeVisible(levelBeam1Knob);
    
    levelBeam2Knob.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    levelBeam2Knob.setTextBoxStyle(Slider::TextBoxLeft, false, LABEL_WIDTH, LABEL_HEIGHT);
    levelBeam2Knob.setColour(Slider::thumbColourId, beamColours[1]);
    levelBeam2Knob.setPopupMenuEnabled(true);
    levelBeam2Knob.setCallback(&processor, "levelBeam2");
    addAndMakeVisible(levelBeam2Knob);
    
    levelBeam1KnobAttachment.reset(new SliderAttachment(valueTreeState, "levelBeam1", levelBeam1Knob));
    levelBeam2KnobAttachment.reset(new SliderAttachment(valueTreeState, "levelBeam2", levelBeam2Knob));
    
    //==============================================================================
    
    muteLabel.setText("MUTE", NotificationType::dontSendNotification);
    muteLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(muteLabel);
    
    muteBeam1Button.setButtonText("1");
    muteBeam1Button.setCallback(&processor, "muteBeam1");
    addAndMakeVisible(muteBeam1Button);
    
    muteBeam2Button.setButtonText("2");
    muteBeam2Button.setCallback(&processor, "muteBeam2");
    addAndMakeVisible(muteBeam2Button);
    
    beam1MuteButtonAttachment.reset(new ButtonAttachment(valueTreeState, "muteBeam1", muteBeam1Button));
    beam2MuteButtonAttachment.reset(new ButtonAttachment(valueTreeState, "muteBeam2", muteBeam2Button));
    
    getLookAndFeel().setColour(MuteButton::buttonOnColourId, Colours::darkred);
    
    //==============================================================================
    
    beam1Meter.setCallback(&processor, 1, 0);
    beam1Meter.startTimerHz(BEAM_METER_UPDATE_FREQ);
    addAndMakeVisible(beam1Meter);
    
    beam2Meter.setCallback(&processor, 1, 1);
    beam2Meter.startTimerHz(BEAM_METER_UPDATE_FREQ);
    addAndMakeVisible(beam2Meter);
    
    //==============================================================================
    
    hpfLabel.setText("HPF", NotificationType::dontSendNotification);
    hpfLabel.setJustificationType(Justification::left);
    hpfLabel.attachToComponent(&hpfSlider, true);
    
    hpfSlider.setSliderStyle(Slider::LinearHorizontal);
    hpfSlider.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    hpfSlider.setPopupMenuEnabled(true);
    hpfSlider.setCallback(&processor, "hpf");
    addAndMakeVisible(hpfSlider);
    
    hpfSliderAttachment.reset(new SliderAttachment(valueTreeState, "hpf", hpfSlider));
    
    //==============================================================================
    
    inputMeter.setCallback(&processor, 0);
    inputMeter.startTimerHz(INPUT_METER_UPDATE_FREQ);
    addAndMakeVisible(inputMeter);
    
    //==============================================================================
    
    gainLabel.setText("GAIN", NotificationType::dontSendNotification);
    gainLabel.setJustificationType(Justification::left);
    gainLabel.attachToComponent(&gainSlider, true);
    
    gainSlider.setSliderStyle(Slider::LinearHorizontal);
    gainSlider.setTextBoxStyle(Slider::TextBoxRight, false, LABEL_WIDTH, LABEL_HEIGHT);
    gainSlider.setPopupMenuEnabled(true);
    gainSlider.setCallback(&processor, "gainMic");
    addAndMakeVisible(gainSlider);
    
    gainSliderAttachment.reset(new SliderAttachment(valueTreeState, "gainMic", gainSlider));
    
    //=====================================================
    // Add CPU Load and start its timer
    cpuLoad.setSource(&processor);
    cpuLoad.startTimerHz(CPULOAD_UPDATE_FREQ);
    addAndMakeVisible(cpuLoad);
    
    //=====================================================
    // Add front facing toggle
    frontToggleLabel.setText("FRONT", NotificationType::dontSendNotification);
    frontToggleLabel.attachToComponent(&frontToggle, true);
    frontToggle.setCallback(&processor, "frontFacing");
    addAndMakeVisible(frontToggle);
    
    frontToggleAttachment.reset(new ButtonAttachment(valueTreeState, "frontFacing", frontToggle));
    
    //=====================================================
    // Configuration selection combo
    configComboLabel.setText("SETUP", NotificationType::dontSendNotification);
    configComboLabel.attachToComponent(&configCombo, true);
    configCombo.addItemList(micConfigLabels, 10);
    addAndMakeVisible(configCombo);
    
    configComboLabelAttachment.reset(new ComboBoxAttachment(valueTreeState, "config", configCombo));
    
    /* The editor needs to change its layout when the config changes */
    valueTreeState.addParameterListener("config", this);
    valueTreeState.addParameterListener("frontFacing", this);
    
    //==============================================================================
    /* OSC */
    oscStatus.setColours(processor.isOscReady() ? Colours::green : Colours::red,Colours::grey);
    oscPortLabel.setText("OSC PORT", dontSendNotification);
    oscPortLabel.attachToComponent(&oscPort, true);
    oscPort.setText(String(processor.getOscPort()));
    oscPort.setEnabled(false);
    addAndMakeVisible(oscPort);
    addAndMakeVisible(oscStatus);
    oscStatus.setCallback(&processor, 1);
    oscStatus.startTimerHz(OSC_STATUS_UPDATE_FREQ);
    
}

EBeamerAudioProcessorEditor::~EBeamerAudioProcessorEditor() {
    valueTreeState.removeParameterListener("config", this);
    valueTreeState.removeParameterListener("frontFacing", this);
}

//==============================================================================

void EBeamerAudioProcessorEditor::paint(Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
    g.setColour(Colours::white);
    g.setFont(15.0f);
    
    auto versionArea = getBounds().removeFromBottom(12);
    versionArea.removeFromBottom(2);
    g.setColour(Colours::lightgrey);
    g.setFont(12);
    g.drawText("ISPL and Eventide - Ebeamer v" + String(JucePlugin_VersionString), versionArea,
               Justification::centredBottom, false);
    
}

void EBeamerAudioProcessorEditor::resized() {
    
    auto area = getLocalBounds();
    area.removeFromTop(TOP_BOTTOM_MARGIN);
    area.removeFromBottom(TOP_BOTTOM_MARGIN);
    
    auto sceneArea = area.removeFromTop(SCENE_HEIGHT);
    
    if (isLinearArray(static_cast<MicConfig>((int)*valueTreeState.getRawParameterValue("config")))){
        steerBeamY1Slider.setVisible(false);
        steerBeamY2Slider.setVisible(false);
        sceneArea.removeFromRight((area.getWidth() - SCENE_WIDTH) / 2);
        sceneArea.removeFromLeft((area.getWidth() - SCENE_WIDTH) / 2);
        scene.setBounds(sceneArea);
    }else{
        steerBeamY1Slider.setVisible(true);
        steerBeamY2Slider.setVisible(true);
        sceneArea.removeFromLeft(15);
        sceneArea.removeFromRight(15);
        steerBeamY1Slider.setBounds(sceneArea.removeFromLeft(50));
        steerBeamY2Slider.setBounds(sceneArea.removeFromRight(50));
        sceneArea.removeFromLeft(5);
        sceneArea.removeFromRight(5);
        sceneArea.removeFromTop(10);
        sceneArea.removeFromBottom(10);
        scene.setBounds(sceneArea);
    }
    
    area.removeFromLeft(LEFT_RIGHT_MARGIN);
    area.removeFromRight(LEFT_RIGHT_MARGIN);
    
    
    area.removeFromTop(STEER_SLIDER_TOP_MARGIN);
    steerBeamX1Slider.setBounds(area.removeFromTop(STEER_SLIDER_HEIGHT).withTrimmedLeft(LABEL_BEAM_WIDTH));
    steerBeamX2Slider.setBounds(area.removeFromTop(STEER_SLIDER_HEIGHT).withTrimmedLeft(LABEL_BEAM_WIDTH));
    
    steerLabel.setBounds(area.removeFromTop(LABEL_HEIGHT));
    
    area.removeFromLeft(KNOBS_LEFT_RIGHT_MARGIN);
    area.removeFromRight(KNOBS_LEFT_RIGHT_MARGIN);
    
    auto knobsArea = area.removeFromTop(KNOB_HEIGHT + KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    widthBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    widthBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    widthLabel.setBounds(knobsArea);
    
    knobsArea = area.removeFromTop(KNOB_HEIGHT + KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    panBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    panBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    panLabel.setBounds(knobsArea);
    
    knobsArea = area.removeFromTop(KNOB_HEIGHT + KNOB_TOP_MARGIN);
    knobsArea.removeFromTop(KNOB_TOP_MARGIN);
    levelBeam1Knob.setBounds(knobsArea.removeFromLeft(KNOB_WIDTH));
    levelBeam2Knob.setBounds(knobsArea.removeFromRight(KNOB_WIDTH));
    auto meterArea = knobsArea.removeFromLeft(BEAM_LED_WIDTH + BEAM_LEFT_RIGHT_MARGIN);
    meterArea.removeFromTop(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromBottom(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromLeft(BEAM_LEFT_RIGHT_MARGIN);
    beam1Meter.setBounds(meterArea.removeFromLeft(BEAM_LED_WIDTH));
    meterArea = knobsArea.removeFromRight(BEAM_LED_WIDTH + BEAM_LEFT_RIGHT_MARGIN);
    meterArea.removeFromTop(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromBottom(BEAM_TOP_BOTTOM_MARGIN);
    meterArea.removeFromRight(BEAM_LEFT_RIGHT_MARGIN);
    beam2Meter.setBounds(meterArea.removeFromRight(BEAM_LED_WIDTH));
    levelLabel.setBounds(knobsArea);
    
    auto mutesArea = area.removeFromTop(MUTE_HEIGHT + MUTE_TOP_MARGIN);
    mutesArea.removeFromTop(MUTE_TOP_MARGIN);
    mutesArea.removeFromLeft(MUTE_LEFT_RIGHT_MARGIN);
    mutesArea.removeFromRight(MUTE_LEFT_RIGHT_MARGIN);
    muteBeam1Button.setBounds(mutesArea.removeFromLeft(MUTE_WIDTH));
    muteBeam2Button.setBounds(mutesArea.removeFromRight(MUTE_WIDTH));
    muteLabel.setBounds(mutesArea);
    
    area.removeFromTop(INPUT_SECTION_TOP_MARGIN);
    hpfSlider.setBounds(area.removeFromTop(INPUT_HPF_SLIDER_HEIGHT).withTrimmedLeft(INPUT_HPF_LABEL_WIDTH));
    
    auto inputLedArea = area.removeFromTop(INPUT_LED_HEIGHT);
    inputLedArea.removeFromLeft(INPUT_LEFT_RIGHT_MARGIN+4);
    inputLedArea.removeFromRight(INPUT_LEFT_RIGHT_MARGIN+23);
    inputMeter.setBounds(inputLedArea);
    
    gainSlider.setBounds(area.removeFromTop(INPUT_GAIN_SLIDER_HEIGHT).withTrimmedLeft(INPUT_GAIN_LABEL_WIDTH));
    
    //===============================================================
    /** Prepare area for the footer */
    area.removeFromTop(FOOTER_MARGIN);
    auto footerArea = area.removeFromTop(FOOTER_HEIGHT);
    
    /** Set area for CPU Load */
    cpuLoad.setBounds(footerArea.removeFromLeft(CPULOAD_WIDTH));
    
    /** OSC Port and activity LED */
    footerArea.removeFromLeft(OSC_PORT_LABEL_WIDTH);
    oscPort.setBounds(footerArea.removeFromLeft(OSC_PORT_WIDTH));
    
    footerArea.removeFromLeft(MEDIUM_MARGIN);
    oscStatus.setBounds(footerArea.removeFromLeft(OSC_LED_SIZE));
    
    /** Set area for config combo */
    footerArea.removeFromLeft(CONFIG_COMBO_LABEL_WIDTH);
    configCombo.setBounds(footerArea.removeFromLeft(CONFIG_COMBO_WIDTH));
    
    /** Set area for front toggle */
    footerArea.removeFromLeft(FRONT_TOGGLE_LABEL_WIDTH);
    frontToggle.setBounds(footerArea.removeFromLeft(FRONT_TOGGLE_WIDTH));
}

void EBeamerAudioProcessorEditor::parameterChanged (const String & parameterID, float newValue){
    if (parameterID == "config"){
        resized();
    }
    if (parameterID == "frontFacing"){
        scene.resized();
    }
}

