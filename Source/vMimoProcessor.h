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

#define FIR_LEN 512
#define NUM_BUFFERS 1
#define BEAMSTEERING_ALG_IDEAL

class vMimoProcessor{
    
public:
    
    vMimoProcessor(int samplesPerBlock);
    
    //==============================================================================
    
    // DOA filters, ready for convolution
    std::vector<vFIR::AudioBufferFFT> firSteeringFFT;

    // FIR filters, ready for convolution
    std::vector<vFIR::AudioBufferFFT> firBeamwidthFFT;
    
    //==============================================================================
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
    vFIR::AudioBufferFFT fftBuffers; // numChannels = NUM_BUFFERS; numSamples = 2*fftSize
    
    //==============================================================================
    int samplesPerBlock;
    
};
