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

    float get(int ch) const;

    class Callback {
    public:
        virtual ~Callback() = default;

        virtual float getMeterValue(int meterId, int channel) const = 0;

        virtual void getMeterValues(std::vector<float> &values, int meterId) const = 0;
    };

private:

    std::vector<int> idxs;
    std::vector<std::vector<float>> minMaxCircularBuffer;

    SpinLock minMaxCircularBufferLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterDecay)
};

float panToLinearGain(float gain, bool isLeftChannel);
