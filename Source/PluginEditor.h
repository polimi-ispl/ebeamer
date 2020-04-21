#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SceneComponent.h"
#include "AudioComponents.h"
#include "CpuLoadComp.h"

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
#define INPUT_LEFT_RIGHT_MARGIN 100
#define INPUT_LED_HEIGHT 5
#define INPUT_GAIN_SLIDER_HEIGHT 40
#define INPUT_GAIN_LABEL_WIDTH 50

#define PREFORMANCE_MONITOR_HEIGHT 20
#define CPULOAD_WIDTH 80
#define CPULOAD_UPDATE_FREQ 10 //Hz

#define FRONT_TOGGLE_LABEL_WIDTH 65
#define FRONT_TOGGLE_WIDTH 25

#define INPUT_METER_UPDATE_FREQ 10 //Hz
#define BEAM_METER_UPDATE_FREQ 10 //Hz
#define ENERGY_UPDATE_FREQ 10 //Hz


//==============================================================================

class JucebeamAudioProcessorEditor  : public AudioProcessorEditor,
                                      private ToggleButton::Listener,
                                      private Slider::Listener
{
public:

    JucebeamAudioProcessorEditor (EbeamerAudioProcessor&);
    ~JucebeamAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    
    EbeamerAudioProcessor& processor;

    // Project specific

    SceneComponent scene;
    
    Label labelBeam1;
    Label labelBeam2;

    Slider steeringBeam1Slider;
    Slider steeringBeam2Slider;

    Label steerLabel;

    Label widthLabel;
    Slider widthBeam1Knob;
    Slider widthBeam2Knob;

    Label panLabel;
    PanSlider panBeam1Knob;
    PanSlider panBeam2Knob;

    Label levelLabel;
    DecibelSlider levelBeam1Knob;
    DecibelSlider levelBeam2Knob;

    Label muteLabel;
    TextButton beam1MuteButton;
    TextButton beam2MuteButton;
    
    MultiChannelLedBar inputMeter;
    SingleChannelLedBar beam1Meter;
    SingleChannelLedBar beam2Meter;
    
    Label hpfLabel;
    FrequencySlider hpfSlider;
    
    Label gainLabel;
    DecibelSlider gainSlider;
    
    //===============================================================
    /** CPU load component */
    CpuLoadComp cpuLoad;
    
    /** Swap side toggle component */
    Label frontToggleLabel;
    ToggleButton frontToggle;

    //===============================================================
    void setMuteButtonColor(uint8 beamIdx);
    
    // Appearance
    const std::vector<Colour> beamColours = {Colours::orangered,Colours::royalblue};
    
    // Callbacks
    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;
    void buttonStateChanged(Button *button) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
