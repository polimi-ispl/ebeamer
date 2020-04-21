/*
  ==============================================================================

    CpuLoadComp.h
    Created: 18 Apr 2020 4:23:20pm
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class CpuLoadComp    : public Component,
                       public Timer
{
public:
    CpuLoadComp(const EbeamerAudioProcessor &p);
    ~CpuLoadComp();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuLoadComp)
    
    /** Load indicator text */
    TextEditor text;
    /** Label for load indicator text */
    Label label;
    /** Timer callback */
    void timerCallback() override;
    
    /** Processor instance */
    const EbeamerAudioProcessor &processor;
    
    // Constants
    const float textHeight = 10;
    const float labelWidth = 50;
    
};
