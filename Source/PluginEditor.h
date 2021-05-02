/*
 eBeamer Plugin Processor GUI
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

class EBeamerAudioProcessorEditor :
public AudioProcessorEditor,
public EbeamerGUI::Callback
{
public:
    
    EBeamerAudioProcessorEditor(EbeamerAudioProcessor &, ValueTree vt);
    
    ~EBeamerAudioProcessorEditor();
    
    void paint(Graphics &) override;
    
    void resized() override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EBeamerAudioProcessorEditor);

    EbeamerGUI gui;

};
