/*
  ==============================================================================

    vFIR.cpp
    Created: 27 Apr 2019 9:45:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "vFIR.h"


namespace vFIR{
    
    void readFIR(std::vector<AudioBuffer<float>>& firTimeSeries, const char* array,const int len) {
        MemoryInputStream inputStream(array, len,false);
        uint32 numFilters;
        inputStream.read(&numFilters, 4);
        uint32 numChannels;
        inputStream.read(&numChannels, 4);
        uint32 filtersLen;
        inputStream.read(&filtersLen, 4);
        uint32 fs;
        inputStream.read(&fs, 4);
        
        float value;
        firTimeSeries.resize(numFilters);
        for (auto filterIdx = 0; filterIdx < numFilters; ++filterIdx){
            firTimeSeries[filterIdx] = AudioBuffer<float>(numChannels,filtersLen);
            for (auto channelIdx = 0; channelIdx < numChannels; ++channelIdx){
                for (auto coeffIdx = 0; coeffIdx < filtersLen; ++coeffIdx){
                    inputStream.read(&value,4);
                    firTimeSeries[filterIdx].setSample(channelIdx, coeffIdx, value);
                }
            }
        }
    }
    
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
        buffer = AudioBuffer<float>(numChannels,fft->getSize()*2);
    }
    
    AudioBufferFFT::AudioBufferFFT(const AudioBuffer<float>& in_,std::shared_ptr<dsp::FFT>&fft_){
        fft = fft_;
        convBuffer = AudioBuffer<float>(1,fft->getSize()*2);
        buffer = AudioBuffer<float>(in_.getNumChannels(),fft->getSize()*2);
        setTimeSeries(in_);
    }
    
    void AudioBufferFFT::setTimeSeries(const AudioBuffer<float> & in_){
        if (fft->getSize() < in_.getNumSamples()){
            throw std::runtime_error("fft->too small for the provided input signal");
        }
        
        buffer.clear();
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            buffer.copyFrom(channelIdx, 0, in_, channelIdx, 0, in_.getNumSamples());
        }
        // perform FFT
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            fft->performRealOnlyForwardTransform(buffer.getWritePointer(channelIdx));
        }
    }
    
    void AudioBufferFFT::updateSymmetricFrequency() {
        if (readyForConvolution){
            for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
                updateSymmetricFrequencyDomainData(buffer.getWritePointer(channelIdx), fft->getSize());
            }
            readyForConvolution = false;
        }
    }

    void AudioBufferFFT::getTimeSeries(AudioBuffer<float>& out){
        updateSymmetricFrequency();
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            convBuffer.copyFrom(0, 0, buffer, channelIdx, 0, fft->getSize()*2);
            fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
            out.copyFrom(channelIdx, 0, convBuffer, 0, 0, fft->getSize());
        }
    }
    
    void AudioBufferFFT::addTimeSeries(AudioBuffer<float>& out){
        updateSymmetricFrequency();
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            convBuffer.copyFrom(0, 0, buffer, channelIdx, 0, fft->getSize()*2);
            fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
            out.addFrom(channelIdx, 0, convBuffer, 0, 0, fft->getSize());
        }
    }
    
    void AudioBufferFFT::getTimeSeries(int sourceCh, AudioBuffer<float>& dest,int destCh){
        updateSymmetricFrequency();
        convBuffer.copyFrom(0, 0, buffer, sourceCh, 0, fft->getSize()*2);
        fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
        dest.copyFrom(destCh, 0, convBuffer, 0, 0, fft->getSize());
    }
    
    void AudioBufferFFT::addTimeSeries(int sourceCh, AudioBuffer<float>& dest,int destCh){
        updateSymmetricFrequency();
        convBuffer.copyFrom(0, 0, buffer, sourceCh, 0, fft->getSize()*2);
        fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
        dest.addFrom(destCh, 0, convBuffer, 0, 0, fft->getSize());
    }
    
    void AudioBufferFFT::prepareForConvolution() {
        if (! readyForConvolution){
            for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
                prepareForConvolution(buffer.getWritePointer(channelIdx), fft->getSize());
            }
            readyForConvolution = true;
        }
    }

    float* AudioBufferFFT::getConvReady(int channelIdx){
        prepareForConvolution();
        return buffer.getWritePointer(channelIdx);
    }

    void AudioBufferFFT::convolve(int outputChannel, const AudioBufferFFT& in_, int inChannel, AudioBufferFFT& filter_, int filterChannel ){
        
        if (!in_.isReadyForConvolution()){
            throw std::runtime_error("Input must be ready for convolution");
        }
        if (!filter_.isReadyForConvolution()){
            throw std::runtime_error("Filter must be ready for convolution");
        }
       
        buffer.clear(outputChannel,0,buffer.getNumSamples());
        convolutionProcessingAndAccumulate(in_.buffer.getReadPointer(inChannel),filter_.buffer.getReadPointer(filterChannel),buffer.getWritePointer(outputChannel),fft->getSize());
        
        readyForConvolution = true;
        
    }
    
    void AudioBufferFFT::copyFrom(int dstChannel, int dstStartSample, const AudioBufferFFT& inFFT, int inChannel, int inStartSample, size_t numSamples){
        buffer.copyFrom(dstChannel, dstStartSample, inFFT.buffer, inChannel, inStartSample, static_cast<int>(numSamples));
    }
    
} // End namespace vFIR

