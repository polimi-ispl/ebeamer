/*
  ==============================================================================

    vFIR.cpp
    Created: 27 Apr 2019 9:45:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "vFIR.h"


namespace vFIR{
    //========== copied from juce_Convolution.cpp ============

    /** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
    void prepareForConvolution (float *samples, int fftSize) noexcept
    {
        int FFTSizeDiv2 = fftSize / 2;
        
        for (size_t i = 0; i < FFTSizeDiv2; i++)
            samples[i] = samples[2 * i];
        
        samples[FFTSizeDiv2] = 0;
        
        for (size_t i = 1; i < FFTSizeDiv2; i++)
            samples[i + FFTSizeDiv2] = -samples[2 * (fftSize - i) + 1];
    }

    /** Does the convolution operation itself only on half of the frequency domain samples. */
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize)
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
    void updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept
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

    std::vector<std::vector<std::vector<float> > > readFIR(const char* array,const int len) {
        MemoryInputStream inputStream(array, len,false);
        uint32 numFilters;
        inputStream.read(&numFilters, 4);
        uint32 numChannels;
        inputStream.read(&numChannels, 4);
        uint32 filtersLen;
        inputStream.read(&filtersLen, 4);
        uint32 fs;
        inputStream.read(&fs, 4);
        
        std::vector<std::vector<std::vector<float>>> fir(numFilters);
        for (auto filterIdx = 0; filterIdx < numFilters; ++filterIdx){
            fir[filterIdx].resize(numChannels);
            for (auto channelIdx = 0; channelIdx < numChannels; ++channelIdx){
                fir[filterIdx][channelIdx].resize(filtersLen);
                for (auto coeffIdx = 0; coeffIdx < filtersLen; ++coeffIdx){
                    inputStream.read(&(fir[filterIdx][channelIdx][coeffIdx]),4);
                }
            }
        }
        return fir;
    }

    AudioBufferFFT::AudioBufferFFT(const AudioBuffer<float>& in_,std::shared_ptr<dsp::FFT>&fft_){
        /**
         Given a time signal computes the fft->for each channel
         */
        fft = fft_;
        if (fft->getSize() < in_.getNumSamples()){
            throw std::runtime_error("fft->too small for the provided input signal");
        }
        convBuffer = AudioBuffer<SampleType>(1,fft->getSize()*2);
        
        // initialize and copy from in_ to buffer
        buffer = AudioBuffer<SampleType>(in_.getNumChannels(),fft->getSize()*2);
        buffer.clear();
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            buffer.copyFrom(channelIdx, 0, in_, channelIdx, 0, in_.getNumSamples());
        }
        // perform FFT
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            fft->performRealOnlyForwardTransform(buffer.getWritePointer(channelIdx));
        }
    }
    
    void AudioBufferFFT::getTimeSeries(AudioBuffer<float>& out){
        out.clear();
        
        if (readyForConvolution){
            for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
                updateSymmetricFrequencyDomainData(buffer.getWritePointer(channelIdx), fft->getSize());
            }
            readyForConvolution = false;
        }
        
        for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
            convBuffer.copyFrom(0, 0, buffer, channelIdx, 0, fft->getSize()*2);
            fft->performRealOnlyInverseTransform(convBuffer.getWritePointer(0));
            out.copyFrom(channelIdx, 0, convBuffer, 0, 0, fft->getSize());
        }
        
    }
    
    float* AudioBufferFFT::getConvReady(int channelIdx){
        if (! readyForConvolution){
            for (int channelIdx = 0; channelIdx < buffer.getNumChannels(); ++channelIdx){
                prepareForConvolution(buffer.getWritePointer(channelIdx), fft->getSize());
            }
            readyForConvolution = true;
        }
        return buffer.getWritePointer(channelIdx);
    }
    
    void AudioBufferFFT::convolve(int signalChannel, const AudioBufferFFT& filter, int filterChannel, AudioBufferFFT& output, int outputChannel){
        convolutionProcessingAndAccumulate(buffer.getReadPointer(signalChannel),filter.getReadPointer(filterChannel),output.getWritePointer(outputChannel),fft->getSize());
    }

    
} // End namespace vFIR

