/*
 Audio Buffer in FFT domain
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioBufferFFT : public AudioBuffer<float> {

public:
    AudioBufferFFT() {};

    AudioBufferFFT(int numChannels, std::shared_ptr<dsp::FFT> &);

    AudioBufferFFT(const AudioBuffer<float> &, std::shared_ptr<dsp::FFT> &);

    void reset();

    void setTimeSeries(const AudioBuffer<float> &);

    void copyToTimeSeries(AudioBuffer<float> &);

    void copyToTimeSeries(int sourceCh, AudioBuffer<float> &dest, int destCh);

    void addToTimeSeries(AudioBuffer<float> &);

    void addToTimeSeries(int sourceCh, AudioBuffer<float> &dest, int destCh);

    void
    convolve(int outputChannel, const AudioBufferFFT &in_, int inChannel, AudioBufferFFT &filter_, int filterChannel);

    void prepareForConvolution();

    bool isReadyForConvolution() const { return readyForConvolution; };

private:
    AudioBuffer<float> convBuffer;
    std::shared_ptr<dsp::FFT> fft;

    void prepareForConvolution(float *samples, int fftSize) const;

    void convolutionProcessingAndAccumulate(const float *input, const float *impulse, float *output, int fftSize) const;

    void updateSymmetricFrequencyDomainData(float *samples, int fftSize) const;

    void updateSymmetricFrequency();

    bool readyForConvolution = false;

};

