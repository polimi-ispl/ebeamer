/*
 ==============================================================================
 
 vFIR.cpp
 Created: 27 Apr 2019 9:45:28am
 Author:  Luca Bondi
 
 ==============================================================================
 */

#include "FIR.h"


namespace FIR{

//========== copied from juce_Convolution.cpp ============

/** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
void AudioBufferFFT::prepareForConvolution (float *samples, int fftSize) noexcept
{
    int FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[2 * i];
    
    samples[FFTSizeDiv2] = 0;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[2 * (fftSize - i) + 1];
}

/** Does the convolution operation itself only on half of the frequency domain samples. */
void AudioBufferFFT::convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize)
{
    int FFTSizeDiv2 = fftSize / 2;
    
    FloatVectorOperations::addWithMultiply      (output, input, impulse, FFTSizeDiv2);
    FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), FFTSizeDiv2);
    
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), FFTSizeDiv2);
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, FFTSizeDiv2);
    
    output[fftSize] += input[fftSize] * impulse[fftSize];
}

/** Undo the re-organization of samples from the function prepareForConvolution.
 Then, takes the conjugate of the frequency domain first half of samples, to fill the
 second half, so that the inverse transform will return real samples in the time domain.
 */
void AudioBufferFFT::updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * (fftSize - i)] = samples[i];
        samples[2 * (fftSize - i) + 1] = -samples[FFTSizeDiv2 + i];
    }
    
    samples[1] = 0.f;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * i] = samples[2 * (fftSize - i)];
        samples[2 * i + 1] = -samples[2 * (fftSize - i) + 1];
    }
}

AudioBufferFFT::AudioBufferFFT(int numChannels,std::shared_ptr<dsp::FFT>&fft_){
    fft = fft_;
    convBuffer = AudioBuffer<float>(1,fft->getSize()*2);
    setSize(numChannels, fft->getSize()*2);
}

AudioBufferFFT::AudioBufferFFT(const AudioBuffer<float>& in_,std::shared_ptr<dsp::FFT>&fft_){
    fft = fft_;
    convBuffer = AudioBuffer<float>(1,fft->getSize()*2);
    setSize(in_.getNumChannels(), fft->getSize()*2);
    setTimeSeries(in_);
}

void AudioBufferFFT::reset(){
    clear();
    readyForConvolution=false;
}

void AudioBufferFFT::setTimeSeries(const AudioBuffer<float> & in_){
    jassert(fft->getSize() >= in_.getNumSamples());
    
    clear();
    for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
        copyFrom(channelIdx, 0, in_, channelIdx, 0, in_.getNumSamples());
    }
    // perform FFT
    for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
        fft->performRealOnlyForwardTransform(getWritePointer(channelIdx));
    }
    
    readyForConvolution = false;
}

void AudioBufferFFT::updateSymmetricFrequency() {
    if (readyForConvolution){
        for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
            updateSymmetricFrequencyDomainData(getWritePointer(channelIdx), fft->getSize());
        }
        readyForConvolution = false;
    }
}

void AudioBufferFFT::getTimeSeries(AudioBuffer<float>& out){
    updateSymmetricFrequency();
    for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
        convBuffer.copyFrom(0, 0, *(this), channelIdx, 0, fft->getSize()*2);
        fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
        out.copyFrom(channelIdx, 0, convBuffer, 0, 0, fft->getSize());
    }
}

void AudioBufferFFT::addTimeSeries(AudioBuffer<float>& out){
    updateSymmetricFrequency();
    for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
        convBuffer.copyFrom(0, 0, *(this), channelIdx, 0, fft->getSize()*2);
        fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
        out.addFrom(channelIdx, 0, convBuffer, 0, 0, fft->getSize());
    }
}

void AudioBufferFFT::getTimeSeries(int sourceCh, AudioBuffer<float>& dest,int destCh){
    updateSymmetricFrequency();
    convBuffer.copyFrom(0, 0, *(this), sourceCh, 0, fft->getSize()*2);
    fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
    dest.copyFrom(destCh, 0, convBuffer, 0, 0, fft->getSize());
}

void AudioBufferFFT::addTimeSeries(int sourceCh, AudioBuffer<float>& dest,int destCh){
    updateSymmetricFrequency();
    convBuffer.copyFrom(0, 0, *(this), sourceCh, 0, fft->getSize()*2);
    fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
    dest.addFrom(destCh, 0, convBuffer, 0, 0, fft->getSize());
}

void AudioBufferFFT::prepareForConvolution() {
    if (! readyForConvolution){
        for (int channelIdx = 0; channelIdx < getNumChannels(); ++channelIdx){
            prepareForConvolution(getWritePointer(channelIdx), fft->getSize());
        }
        readyForConvolution = true;
    }else{
        // prepareForConvolution should not be called if
        jassertfalse;
    }
}

void AudioBufferFFT::convolve(int outputChannel, const AudioBufferFFT& in_, int inChannel, AudioBufferFFT& filter_, int filterChannel ){
    
    jassert(in_.isReadyForConvolution());
    jassert(filter_.isReadyForConvolution());
    
    clear(outputChannel,0,getNumSamples());
    convolutionProcessingAndAccumulate(in_.getReadPointer(inChannel),filter_.getReadPointer(filterChannel),getWritePointer(outputChannel),fft->getSize());
    
    readyForConvolution = true;
}

} // End namespace vFIR

