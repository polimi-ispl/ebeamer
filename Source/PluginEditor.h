#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DOAthread.h"
#include "SceneComponent.h"
#include "AudioComponents.h"

#define GUI_WIDTH 540


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
#define INPUT_LED_TOP_MARGIN 20
#define INPUT_LEFT_RIGHT_MARGIN 100
#define INPUT_LED_HEIGHT 5
#define INPUT_METER_UPDATE_FREQ 15 //Hz
#define BEAM_LED_WIDTH 5
#define BEAM_TOP_BOTTOM_MARGIN 10
#define BEAM_LEFT_RIGHT_MARGIN 10
#define BEAM_METER_UPDATE_FREQ 15 //Hz
#define ENERGY_UPDATE_FREQ 15 //Hz

//==============================================================================

class JucebeamAudioProcessorEditor  : public AudioProcessorEditor,
                                      private ToggleButton::Listener,
                                      private Slider::Listener
{
public:

    JucebeamAudioProcessorEditor (JucebeamAudioProcessor&);
    ~JucebeamAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JucebeamAudioProcessor& processor;
    std::unique_ptr<DOAthread> DOAt;

    // Project specific

    SceneComponent scene;

    Slider steeringBeam1Slider;
    Slider steeringBeam2Slider;

    Label steerLabel;

    Label widthLabel;
    Slider widthBeam1Knob;
    Slider widthBeam2Knob;

    Label panLabel;
    Slider panBeam1Knob;
    Slider panBeam2Knob;

    Label gainLabel;
    DecibelSlider gainBeam1Knob;
    DecibelSlider gainBeam2Knob;

    Label muteLabel;
    TextButton beam1MuteButton;
    TextButton beam2MuteButton;
    
    MultiChannelLedBar inputMeter;
    SingleChannelLedBar beam1Meter;
    SingleChannelLedBar beam2Meter;

    void setMuteButtonColor(uint8 beamIdx);
    
    // Callbacks
    void buttonClicked(Button *button) override;
    void sliderValueChanged(Slider *slider) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessorEditor);
};
