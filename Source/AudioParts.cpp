/*
  ==============================================================================

    AudioParts.cpp
    Created: 22 Mar 2019 8:48:06am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "AudioParts.h"


MeterDecay::MeterDecay(float fs, float duration, float blockSize, int numChannels)
{
    
    int numBlocks = ceil(duration*fs/blockSize);
    
    minMaxCircularBuffer.resize(numChannels);
    idxs.resize(numChannels);
    for (auto channelIdx = 0; channelIdx < numChannels; ++channelIdx)
    {
        idxs[channelIdx] = 0;
        minMaxCircularBuffer[channelIdx].resize(numBlocks);
    }
    
}

void MeterDecay::push(const AudioBuffer<float>& signal)
{
    for (auto channelIdx = 0; channelIdx < signal.getNumChannels(); ++channelIdx)
    {
        Range<float> minMax = FloatVectorOperations::findMinAndMax(signal.getReadPointer(channelIdx),signal.getNumSamples());
        float maxAbs = jmax(abs(minMax.getStart()),abs(minMax.getEnd()));
        minMaxCircularBuffer[channelIdx][idxs[channelIdx]++] = maxAbs;
        if (idxs[channelIdx] >= minMaxCircularBuffer[channelIdx].size()){
            idxs[channelIdx] = 0;
        }
    }
}

std::vector<float> MeterDecay::get()
{
    std::vector<float> values(minMaxCircularBuffer.size());
    for (auto channelIdx = 0; channelIdx < minMaxCircularBuffer.size(); ++channelIdx)
    {
        float maxVal = 0;
        for (auto idx = 0; idx < minMaxCircularBuffer[channelIdx].size(); ++idx)
        {
            maxVal = jmax(maxVal,minMaxCircularBuffer[channelIdx][idx]);
        }
        values[channelIdx] = maxVal;
    }
    return values;
}
