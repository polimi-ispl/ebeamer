/*
  Meter Decay
 A signal meter with decay
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


class MeterDecay {

public:

    MeterDecay(float fs, float duration, float blockSize, int numChannels);

    void push(const AudioBuffer<float> &signal);

    void get(std::vector<float> &meter) const;
    
    MemoryBlock get() const;

    float get(int ch) const;

private:

    std::vector<int> idxs;
    std::vector<std::vector<float>> minMaxCircularBuffer;

    SpinLock minMaxCircularBufferLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterDecay)
};

float panToLinearGain(float gain, bool isLeftChannel);
