/*
  Meter Decay
 A signal meter with decay 
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include "MeterDecay.h"


MeterDecay::MeterDecay(float fs, float duration, float blockSize, int numChannels) {

    int numBlocks = (int) ceil(duration * fs / blockSize);

    minMaxCircularBuffer.resize(numChannels);
    idxs.resize(numChannels);
    for (auto channelIdx = 0; channelIdx < numChannels; ++channelIdx) {
        idxs[channelIdx] = 0;
        minMaxCircularBuffer[channelIdx].resize(numBlocks);
    }

}

void MeterDecay::push(const AudioBuffer<float> &signal) {
    GenericScopedLock<SpinLock> l(minMaxCircularBufferLock);

    for (auto channelIdx = 0; channelIdx < jmin((int)signal.getNumChannels(),(int)minMaxCircularBuffer.size()); ++channelIdx) {
        Range<float> minMax = FloatVectorOperations::findMinAndMax(signal.getReadPointer(channelIdx),
                                                                   signal.getNumSamples());
        float maxAbs = jmax(abs(minMax.getStart()), abs(minMax.getEnd()));
        minMaxCircularBuffer[channelIdx][idxs[channelIdx]++] = maxAbs;
        if (idxs[channelIdx] >= minMaxCircularBuffer[channelIdx].size()) {
            idxs[channelIdx] = 0;
        }
    }
}

void MeterDecay::get(std::vector<float> &values) const {
    GenericScopedLock<SpinLock> l(minMaxCircularBufferLock);
    values.resize(minMaxCircularBuffer.size());
    for (auto channelIdx = 0; channelIdx < minMaxCircularBuffer.size(); ++channelIdx) {
        float maxVal = 0;
        for (auto idx = 0; idx < minMaxCircularBuffer[channelIdx].size(); ++idx) {
            maxVal = jmax(maxVal, minMaxCircularBuffer[channelIdx][idx]);
        }
        values[channelIdx] = maxVal;
    }
}

float MeterDecay::get(int ch) const {
    GenericScopedLock<SpinLock> l(minMaxCircularBufferLock);
    float maxVal = 0;
    for (auto idx = 0; idx < minMaxCircularBuffer[ch].size(); ++idx) {
        maxVal = jmax(maxVal, minMaxCircularBuffer[ch][idx]);
    }
    return maxVal;
}

float panToLinearGain(float gain, bool isLeftChannel) {
    const float db_at0 = -4.5; //How many dB at each channel when pan is centered (0)
    jassert(gain >= -1);
    jassert(gain <= 1);
    float alpha = std::pow(10., (db_at0 / 20.));
    if (isLeftChannel) {
        gain = -gain;
    }
    float y = (0.5 - alpha) * std::pow(gain, 2.) + 0.5 * gain + alpha;
    return y;
}
