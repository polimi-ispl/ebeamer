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

class EBeamerAudioProcessorEditor : public AudioProcessorEditor, public AudioProcessorValueTreeState::Listener {
public:
    
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    
    EBeamerAudioProcessorEditor(EbeamerAudioProcessor &, AudioProcessorValueTreeState &v);
    
    ~EBeamerAudioProcessorEditor();
    
    void paint(Graphics &) override;
    
    void resized() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EBeamerAudioProcessorEditor);
    
    EbeamerAudioProcessor &processor;
    AudioProcessorValueTreeState &valueTreeState;
    
    //==============================================================================
    SceneComp scene;
    
    //==============================================================================
    Label steerLabel;
    Label steerBeam1Label, steerBeam2Label;
    SliderCC steerBeamX1Slider, steerBeamX2Slider;
    SliderCC steerBeamY1Slider, steerBeamY2Slider;
    std::unique_ptr<SliderAttachment> steerBeamX1SliderAttachment, steerBeamX2SliderAttachment;
    std::unique_ptr<SliderAttachment> steerBeamY1SliderAttachment, steerBeamY2SliderAttachment;
    
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
    
    //==============================================================================
    /** Listener for parameter changes that requre a broad Editor change */
    void parameterChanged (const String & parameterID, float newValue) override;
    
};
