/*
  eBeamer Plugin Processor GUI
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ebeamerDefs.h"
#include "SceneComp.h"
#include "MeterComp.h"
#include "CpuLoadComp.h"
#include "MidiComp.h"

//==============================================================================

class JucebeamAudioProcessorEditor : public AudioProcessorEditor {
public:

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    JucebeamAudioProcessorEditor(EbeamerAudioProcessor &, AudioProcessorValueTreeState &v);

    ~JucebeamAudioProcessorEditor();

    void paint(Graphics &) override;

    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);

    EbeamerAudioProcessor &processor;
    AudioProcessorValueTreeState &valueTreeState;

    //==============================================================================
    SceneComp scene;

    //==============================================================================
    Label steerLabel;
    Label steeringBeam1Label, steeringBeam2Label;
    SliderCC steeringBeam1Slider, steeringBeam2Slider;
    std::unique_ptr<SliderAttachment> steeringBeam1SliderAttachment, steeringBeam2SliderAttachment;

    //==============================================================================
    Label widthLabel;
    SliderCC widthBeam1Knob, widthBeam2Knob;
    std::unique_ptr<SliderAttachment> widthBeam1KnobAttachment, widthBeam2KnobAttachment;

    //==============================================================================
    Label panLabel;
    PanSlider panBeam1Knob, panBeam2Knob;
    std::unique_ptr<SliderAttachment> panBeam1KnobAttachment, panBeam2KnobAttachment;

    //==============================================================================
    Label levelLabel;
    DecibelSlider levelBeam1Knob, levelBeam2Knob;
    std::unique_ptr<SliderAttachment> levelBeam1KnobAttachment, levelBeam2KnobAttachment;

    //==============================================================================
    Label muteLabel;
    MuteButton muteBeam1Button, muteBeam2Button;
    std::unique_ptr<ButtonAttachment> beam1MuteButtonAttachment, beam2MuteButtonAttachment;

    //==============================================================================
    MultiChannelLedBar inputMeter;
    SingleChannelLedBar beam1Meter, beam2Meter;

    //==============================================================================
    Label hpfLabel;
    FrequencySlider hpfSlider;
    std::unique_ptr<SliderAttachment> hpfSliderAttachment;

    //==============================================================================
    Label gainLabel;
    DecibelSlider gainSlider;
    std::unique_ptr<SliderAttachment> gainSliderAttachment;

    //==============================================================================
    /** CPU load component */
    CpuLoadComp cpuLoad;

    //==============================================================================
    /** Swap side toggle component */
    Label frontToggleLabel;
    ToggleButtonCC frontToggle;
    std::unique_ptr<ButtonAttachment> frontToggleAttachment;

    //==============================================================================
    /** Configuration selection combo */

    Label configComboLabel;
    ComboBox configCombo;
    std::unique_ptr<ComboBoxAttachment> configComboLabelAttachment;

    //==============================================================================
    const std::vector<Colour> beamColours = {Colours::orangered, Colours::royalblue};

};
