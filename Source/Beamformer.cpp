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
    
    firIR.resize(numBeams);
    firFFT.resize(numBeams);
    
}

Beamformer::~Beamformer(){
    
}

MicConfig Beamformer::getMicConfig() const{
    return micConfig;
}

void Beamformer::setMicConfig(MicConfig micConfig_){
    if (micConfig_ != micConfig){
        micConfig=micConfig_;
        initAlg();
    }
}

void Beamformer::initAlg(){
    
    /** Determine configuration parameters */
    int numMic;
    switch (micConfig){
        case LMA_1ESTICK:
            numMic = 16;
            break;
        case LMA_2ESTICK:
            numMic = 32;
            break;
        case LMA_3ESTICK:
            numMic = 48;
            break;
        case LMA_4ESTICK:
            numMic = 64;
            break;
    }
    /** Distance between microphones in eSticks*/
    const float micDist = 0.03;
    alg = std::make_unique<DAS::FarfieldLMA>(micDist,numMic,sampleRate,soundspeed);
    
    firLen = alg->getFirLen();
    
    /** Create shared FFT object */
    fft = std::make_shared<juce::dsp::FFT>(ceil(log2(firLen+maximumExpectedSamplesPerBlock-1)));
    
    /** Allocate FIR filters */
    for (auto &f : firIR){
        f = AudioBuffer<float>(numActiveInputChannels,firLen);
    }
    for (auto &f : firFFT){
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

void Beamformer::prepareToPlay(double sampleRate_, int maximumExpectedSamplesPerBlock_, int numActiveInputChannels_){
    
    sampleRate = sampleRate_;
    maximumExpectedSamplesPerBlock = maximumExpectedSamplesPerBlock_;
    numActiveInputChannels = numActiveInputChannels_;
    
    initAlg();
}

void Beamformer::setBeamParameters(int beamIdx, const BeamParameters& beamParams){
    alg->getFir(firIR[beamIdx], beamParams,alpha);
    firFFT[beamIdx].setTimeSeries(firIR[beamIdx]);
    firFFT[beamIdx].prepareForConvolution();
}

void Beamformer::processBlock(const AudioBuffer<float> &inBuffer){
    
    /** Compute inputs FFT */
    inputBuffer.setTimeSeries(inBuffer);
    inputBuffer.prepareForConvolution();
    
    for (auto beamIdx=0;beamIdx<numBeams;beamIdx++){
        for (auto inCh=0;inCh<inputBuffer.getNumChannels();inCh++){
            /** Convolve inputs and FIR */
            convolutionBuffer.convolve(0, inputBuffer, inCh, firFFT[beamIdx], inCh);
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
        FloatVectorOperations::copy(beamBuffer.getWritePointer(beamIdx), beamBuffer.getReadPointer(beamIdx)+numSplsOut, numSplsShift);
        beamBuffer.clear(beamIdx, numSplsShift, beamBuffer.getNumSamples()-numSplsShift);
    }
}

void Beamformer::releaseResources(){
    
    /** Release FIR filters */
    for (auto &f : firIR){
        f.setSize(0, 0);
    }
    for (auto &f : firFFT){
        f.setSize(0, 0);
    }
    
    /** Release input buffer */
    inputBuffer.setSize(0, 0);
    
    /** Release convolution buffer */
    convolutionBuffer.setSize(0, 0);
    
    /** Release beam buffer */
    beamBuffer.setSize(0, 0);
}
