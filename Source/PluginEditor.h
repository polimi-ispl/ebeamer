#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SceneComponent.h"
#include "AudioComponents.h"
#include "CpuLoadComp.h"
#include "MidiComp.h"

#define GUI_WIDTH 540

#define LABEL_BEAM_WIDTH 25
#define STEER_SLIDER_HEIGHT 40
#define STEER_SLIDER_TOP_MARGIN 10
#define KNOB_WIDTH 150
#define KNOB_HEIGHT 80
#define KNOB_TOP_MARGIN 8
#define MUTE_HEIGHT 40
#define MUTE_WIDTH 40
#define MUTE_LEFT_RIGHT_MARGIN 20
#define MUTE_TOP_MARGIN 8
#define LABEL_WIDTH 70
#define LABEL_HEIGHT 20
#define LEFT_RIGHT_MARGIN 20
#define TOP_BOTTOM_MARGIN 20
#define KNOBS_LEFT_RIGHT_MARGIN 20
#define BEAM_LED_WIDTH 5
#define BEAM_TOP_BOTTOM_MARGIN 10
#define BEAM_LEFT_RIGHT_MARGIN 10

#define INPUT_SECTION_TOP_MARGIN 20
#define INPUT_HPF_SLIDER_HEIGHT 40
#define INPUT_HPF_LABEL_WIDTH 50
#define INPUT_LEFT_RIGHT_MARGIN 50
#define INPUT_LED_HEIGHT 5
#define INPUT_GAIN_SLIDER_HEIGHT 40
#define INPUT_GAIN_LABEL_WIDTH 50

#define PREFORMANCE_MONITOR_HEIGHT 20
#define CPULOAD_WIDTH 80
#define CPULOAD_UPDATE_FREQ 10 //Hz

#define FRONT_TOGGLE_LABEL_WIDTH 40
#define FRONT_TOGGLE_WIDTH 25

#define CONFIG_COMBO_LABEL_WIDTH 65
#define CONFIG_COMBO_WIDTH 80

#define INPUT_METER_UPDATE_FREQ 10 //Hz
#define BEAM_METER_UPDATE_FREQ 10 //Hz
#define ENERGY_UPDATE_FREQ 10 //Hz


//==============================================================================

class JucebeamAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    JucebeamAudioProcessorEditor (EbeamerAudioProcessor&,AudioProcessorValueTreeState& v);
    ~JucebeamAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
    
    EbeamerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;

    //==============================================================================
    SceneComponent scene;
    
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
    const std::vector<Colour> beamColours = {Colours::orangered,Colours::royalblue};

};
