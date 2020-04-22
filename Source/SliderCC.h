/*
  ==============================================================================

    SliderCC.h
    Created: 22 Apr 2020 12:32:05pm
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class SliderCC    : public Slider
{
public:
    SliderCC();
    ~SliderCC();
    
    void setProcessorParamName(EbeamerAudioProcessor* proc, const String & param);
    
    void mouseDown (const MouseEvent&) override;

private:
    
    EbeamerAudioProcessor* processor = nullptr;
    String paramName = "";
    Rectangle<int> popupArea;
    
    void showPopupMenu();
    static void sliderMenuCallback (int result, SliderCC* slider);
    
    bool isLearning() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderCC)
};
