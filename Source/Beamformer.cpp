/*
 ==============================================================================
 
 Beamformer.cpp
 Created: 14 Apr 2020 9:12:12am
 Author:  Luca Bondi
 
 ==============================================================================
 */

#include "Beamformer.h"

Beamformer::Beamformer(const AudioProcessor& p, int numBeams_, int numDoas_):processor(p){
    
    numBeams = numBeams_;
    numDoas = numDoas_;
    
    fir.resize(numBeams);
}

Beamformer::~Beamformer(){
    
}

void Beamformer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock, int numActiveInputChannels){
    
    /** Create shared FFT object */
    fft = std::make_shared<juce::dsp::FFT>(ceil(log2(firLen+maximumExpectedSamplesPerBlock-1)));
    
    /** Allocate FIR filters */
    for (auto &f : fir){
        f = FIR::AudioBufferFFT(numActiveInputChannels,fft);
    }
    
    /** Allocate input buffers */
    inputBuffer = FIR::AudioBufferFFT(numActiveInputChannels, fft);
    
    /** Allocate convolution buffer */
    convolutionBuffer = FIR::AudioBufferFFT(1, fft);
    
    /** Allocate beam output buffer */
    beamBuffer.setSize(numBeams, convolutionBuffer.getNumSamples()/2);
    beamBuffer.clear();
}

void Beamformer::setBeamParameters(int beamIdx, const BeamParameters& beamParams){
    /** Generate the proper FIR filter */
    fir[beamIdx].reset();
    AudioBuffer<float> impulse(fir[beamIdx].getNumChannels(),1);
    for (auto chIdx=0;chIdx<fir[beamIdx].getNumChannels();chIdx++){
        impulse.setSample(chIdx, 0, 1);
    }
    fir[beamIdx].setTimeSeries(impulse);
    fir[beamIdx].prepareForConvolution();
}

void Beamformer::processBlock(const AudioBuffer<float> &inBuffer){
    
    /** Compute inputs FFT */
    inputBuffer.setTimeSeries(inBuffer);
    inputBuffer.prepareForConvolution();
    
    for (auto beamIdx=0;beamIdx<numBeams;beamIdx++){
        for (auto inCh=0;inCh<inputBuffer.getNumChannels();inCh++){
            /** Convolve inputs and FIR */
            convolutionBuffer.convolve(0, inputBuffer, inCh, fir[beamIdx], inCh);
            /** Overlap and add of convolutionBuffer into beamBuffer */
            convolutionBuffer.addTimeSeries(0, beamBuffer, beamIdx);
        }
    }
    
}

void Beamformer::getBeams(AudioBuffer<float>& outBuffer){
    jassert(outBuffer.getNumChannels() == numBeams);
    auto numSplsOut = outBuffer.getNumSamples();
    auto numSplsShift = beamBuffer.getNumSamples() - outBuffer.getNumSamples();
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++){
        /** Copy beamBuffer to outBuffer */
        outBuffer.copyFrom(beamIdx, 0, beamBuffer, beamIdx, 0, numSplsOut);
        /** Shift beamBuffer */
        //beamBuffer.copyFrom(beamIdx, 0, beamBuffer, beamIdx, numSplsOut, numSplsShift);
        FloatVectorOperations::copy(beamBuffer.getWritePointer(beamIdx), beamBuffer.getReadPointer(beamIdx)+numSplsOut, numSplsShift);
        beamBuffer.clear(beamIdx, numSplsShift, beamBuffer.getNumSamples()-numSplsShift);
    }
}

void Beamformer::releaseResources(){
    
    /** Release FIR filters */
    for (auto &f : fir){
        f.setSize(0, 0);
    }
    
    /** Release input buffer */
    inputBuffer.setSize(0, 0);
    
    /** Release convolution buffer */
    convolutionBuffer.setSize(0, 0);
    
    /** Release beam buffer */
    beamBuffer.setSize(0, 0);
}
