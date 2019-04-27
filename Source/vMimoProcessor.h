/*
  ==============================================================================

    vMimoProcessor.h
    Created: 27 Apr 2019 10:19:55am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#define FIR_LEN 512
#define NUM_BEAMS 2
#define NUM_BUFFERS 2
#define BEAMSTEERING_ALG_IDEAL

class vMimoProcessor{
    
public:
    
    class RuntimeException : public std::runtime_error{
    public:
        RuntimeException(char const* const message) throw() : std::runtime_error(message){};
        char const * what() const throw() { return exception::what();};
    };
    
    vMimoProcessor(int samplesPerBlock,int numInputChannels,int numOutputChannels);
    
    //==============================================================================
    std::vector<std::vector<std::vector<float>>> prepareIR(const std::vector<std::vector<std::vector<float>>> fir);
    
    // DOA filters, ready for convolution
    std::vector<std::vector<std::vector<float>>> firSteeringFFT;

    // FIR filters, ready for convolution
    std::vector<std::vector<std::vector<float>>> firBeamwidthFFT;
    
    //==============================================================================
    // FFT
    const int getFftSize(){ return fft->getSize();}
    
    //==============================================================================
    void newBlock();
    void preComputeInputFFT(const AudioBuffer<float>& input);
    void getInputFFT(AudioBuffer<float>& inputFFTcopy);
    void processBlock(int inChannel,int outChannel,int steeringIdx,int beamWidthIdx);
    AudioBuffer<float> getBeams();
    AudioBuffer<float> beamsBuffer; // numChannels = min(numOutputChannels,numBeams); numSamples = fftSize
    
    size_t getNumSteeringFir(){return firSteeringFFT.size();};
    size_t getNumBeamwidthFir(){return firBeamwidthFFT.size();};
    
    const float* getFirSteeringFFT(int steeringIdx,int inChannel){ return firSteeringFFT[steeringIdx][inChannel].data();};
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (vMimoProcessor)
    
    //==============================================================================
    // FFT
    std::unique_ptr<dsp::FFT> fft;
    
    //==============================================================================
    // Buffers
    
    AudioBuffer<float> fftInputs; // numChannels = numInputChannels; numSamples = 2*fftSize
    AudioBuffer<float> fftBuffers; // numChannels = NUM_BUFFERS; numSamples = 2*fftSize
    
    //==============================================================================
    int samplesPerBlock;
    
};
