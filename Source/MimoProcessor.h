/*
  ==============================================================================

    MimoProcessor.h
    Created: 27 Apr 2019 10:19:55am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "FIR.h"

class MimoProcessor{
    
public:
    
    MimoProcessor(int samplesPerBlock);
    
    //==============================================================================
    // DOA filters
    std::vector<FIR::AudioBufferFFT> firSteeringFFT;
    // Beamwidth filters
    std::vector<FIR::AudioBufferFFT> firBeamwidthFFT;
    // FFT
    std::shared_ptr<dsp::FFT> fft;
    
    //==============================================================================
    void newBlock(AudioBuffer<float>& outTimeSeries);
    void processBlock(const AudioBuffer<float>& inTimeSeries, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx);
    void processBlock(const FIR::AudioBufferFFT& inFFT, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx);
    void processBlock(const FIR::AudioBufferFFT& inFFT, int inChannel, FIR::AudioBufferFFT& outFFT, int outChannel, int steeringIdx, int beamWidthIdx);
    
    size_t getNumSteeringFir(){return firSteeringFFT.size();};
    size_t getNumBeamwidthFir(){return firBeamwidthFFT.size();};
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MimoProcessor)
        
    //==============================================================================
    // Buffers
    FIR::AudioBufferFFT fftBuffers;
    int samplesPerBlock;
    
    //==============================================================================
    // Parameters
    const enum {MEASURED,IDEAL} algorithm = IDEAL;
    const int numBuffers = 1;
    
};
