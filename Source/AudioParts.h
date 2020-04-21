/*
  ==============================================================================

    AudioParts.h
    Created: 22 Mar 2019 8:48:06am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class MeterDecay
{
    
public:
    
    MeterDecay(float fs, float duration, float blockSize, int numChannels);
    void push(const AudioBuffer<float>& signal);
    std::vector<float> get();
    
private:
    
    std::vector<int> idxs;
    std::vector<std::vector<float>> minMaxCircularBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterDecay)
};

float panToLinearGain(float gain, bool isLeftChannel) ;
