/*
  ==============================================================================

    vFIR.h
    Created: 27 Apr 2019 9:45:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace vFIR{
    // Convolution operations
    void prepareForConvolution (float *samples, int fftSize) noexcept;
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize);
    
    void updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept;
    std::vector<std::vector<std::vector<float> > > readFIR(const char* array,const int len);
    
    class AudioBufferFFT: public AudioBuffer<float>{
    private:
        AudioBuffer<float> buffer, convBuffer;
        std::shared_ptr<dsp::FFT> fft;
        bool readyForConvolution = false;
        float* getConvReady(int);
    public:
        AudioBufferFFT(const AudioBuffer<float>&,std::shared_ptr<dsp::FFT>&);
        void getTimeSeries(AudioBuffer<float>&);
        void convolve(int signalChannel, const AudioBufferFFT& filter, int filterChannel, AudioBufferFFT& output, int outputChannel);
    };

    
}

