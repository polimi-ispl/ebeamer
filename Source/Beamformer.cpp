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
    
    doaLevels.resize(numDoas,-100);
    doaFirFFT.resize(numDoas);

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
        f.clear();
    }
    for (auto &f : firFFT){
        f = FIR::AudioBufferFFT(numActiveInputChannels,fft);
        f.clear();
    }

    /** Allocate input buffers */
    inputBuffer = FIR::AudioBufferFFT(numActiveInputChannels, fft);
    
    /** Allocate convolution buffer */
    convolutionBuffer = FIR::AudioBufferFFT(1, fft);
    
    /** Allocate beam output buffer */
    beamBuffer.setSize(numBeams, convolutionBuffer.getNumSamples()/2);
    beamBuffer.clear();
    
    /** Allocate DOA beam */
    doaBeam.setSize(1, convolutionBuffer.getNumSamples()/2);
    
    /** Initialize DOA BPF */
    doaBandPassFilter.setCoefficients(IIRCoefficients::makeBandPass(sampleRate, doaBandPassFrequency,doaBandPassQ));
    
    /** Compute FIR for DOA estimation */
    AudioBuffer<float> tmpFir(numActiveInputChannels,firLen);
    BeamParameters tmpBeamParams{0,0};
    for (auto dirIdx = 0;dirIdx < numDoas;dirIdx++){
        tmpBeamParams.doa = -1+(2./(numDoas-1)*dirIdx);
        alg->getFir(tmpFir, tmpBeamParams,1);
        doaFirFFT[dirIdx] = FIR::AudioBufferFFT(numActiveInputChannels,fft);
        doaFirFFT[dirIdx].setTimeSeries(tmpFir);
        doaFirFFT[dirIdx].prepareForConvolution();
    }
    
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
    
    /** Compute DOA levels */
    {
        GenericScopedLock<SpinLock> lock(doaLock);
        if (!newDoaLevelsAvailable){
            for (auto dirIdx = 0; dirIdx < numDoas; dirIdx++){
                doaBeam.clear();
                for (auto inCh=0;inCh<inputBuffer.getNumChannels();inCh++){
                    /** Convolve inputs and DOA FIR */
                    convolutionBuffer.convolve(0, inputBuffer, inCh, doaFirFFT[dirIdx], inCh);
                    convolutionBuffer.addTimeSeries(0, doaBeam, 0);
                }
                doaBandPassFilter.reset();
                doaBandPassFilter.processSamples(doaBeam.getWritePointer(0), doaBeam.getNumSamples());
                auto range = FloatVectorOperations::findMinAndMax(doaBeam.getReadPointer(0), doaBeam.getNumSamples());
                auto maxAbs = jmax(abs(range.getStart()),abs(range.getEnd()));
                auto maxAbsDb = Decibels::gainToDecibels(maxAbs);
                doaLevels[dirIdx] = maxAbsDb;
            }
            
            newDoaLevelsAvailable = true;
        }
    }
    
}

void Beamformer::getBeams(AudioBuffer<float>& outBuffer){
    jassert(outBuffer.getNumChannels() == numBeams);
    auto numSplsOut = outBuffer.getNumSamples();
    auto numSplsShift = beamBuffer.getNumSamples() - numSplsOut;
    AudioBuffer<float> tmp(1,numSplsShift);
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++){
        /** Copy beamBuffer to outBuffer */
        outBuffer.copyFrom(beamIdx, 0, beamBuffer, beamIdx, 0, numSplsOut);
        /** Shift beamBuffer */
        FloatVectorOperations::copy(beamBuffer.getWritePointer(beamIdx), beamBuffer.getReadPointer(beamIdx)+numSplsOut, numSplsShift);
        beamBuffer.clear(beamIdx, numSplsShift, beamBuffer.getNumSamples()-numSplsShift);
    }
}

void Beamformer::getDoaEnergy(std::vector<float>& outDoaLevels){
    jassert(outDoaLevels.size() == doaLevels.size());
    GenericScopedLock<SpinLock> lock(doaLock);
    if (newDoaLevelsAvailable){
        outDoaLevels = doaLevels;
        newDoaLevelsAvailable = false;
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
    for (auto &f : doaFirFFT){
        f.setSize(0, 0);
    }

    /** Release input buffer */
    inputBuffer.setSize(0, 0);
    
    /** Release convolution buffer */
    convolutionBuffer.setSize(0, 0);
    
    /** Release beam buffer */
    beamBuffer.setSize(0, 0);
    
    /** Reset DOA levels */
    for (auto &lev : doaLevels){
        lev = -100;
    }
    newDoaLevelsAvailable = false;
    
    
}
