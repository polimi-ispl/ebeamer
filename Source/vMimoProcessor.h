/*
  ==============================================================================

    vMimoProcessor.h
    Created: 27 Apr 2019 10:19:55am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "vFIR.h"

class vMimoProcessor{
    
public:
    
    vMimoProcessor(int samplesPerBlock);
    
    //==============================================================================
    // DOA filters
    std::vector<vFIR::AudioBufferFFT> firSteeringFFT;
    // Beamwidth filters
    std::vector<vFIR::AudioBufferFFT> firBeamwidthFFT;
    // FFT
    std::shared_ptr<dsp::FFT> fft;
    
    //==============================================================================
    void newBlock(AudioBuffer<float>& outTimeSeries);
    void processBlock(const AudioBuffer<float>& inTimeSeries, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx);
    void processBlock(const vFIR::AudioBufferFFT& inFFT, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx);
    void processBlock(const vFIR::AudioBufferFFT& inFFT, int inChannel, vFIR::AudioBufferFFT& outFFT, int outChannel, int steeringIdx, int beamWidthIdx);
    
    size_t getNumSteeringFir(){return firSteeringFFT.size();};
    size_t getNumBeamwidthFir(){return firBeamwidthFFT.size();};
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vMimoProcessor)
        
    //==============================================================================
    // Buffers
    vFIR::AudioBufferFFT fftBuffers;
    int samplesPerBlock;
    
    //==============================================================================
    // Parameters
    const enum {MEASURED,IDEAL} algorithm = IDEAL;
    const int numBuffers = 1;
    
};
