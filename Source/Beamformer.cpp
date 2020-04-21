/*
 ==============================================================================
 
 Beamformer.cpp
 Created: 14 Apr 2020 9:12:12am
 Author:  Luca Bondi
 
 ==============================================================================
 */

#include "Beamformer.h"

BeamformerDoa::BeamformerDoa(Beamformer& b,
                                         int numDoas_,
                                         float sampleRate_,
                                         int numActiveInputChannels,
                                         int firLen,
                                         std::shared_ptr<dsp::FFT> fft_):beamformer(b){
    
    numDoas = numDoas_;
    fft = fft_;
    sampleRate = sampleRate_;
    
    /** Initialize levels and FIR */
    doaLevels.resize(numDoas,-100);
    doaFirFFT.resize(numDoas);
    
    /** Allocate inputBuffer */
    inputBuffer = FIR::AudioBufferFFT(numActiveInputChannels,fft);
    
    /** Allocate convolution buffer */
    convolutionBuffer = FIR::AudioBufferFFT(1, fft);
    
    /** Allocate DOA beam */
    doaBeam.setSize(1, fft->getSize());
    
    /** Compute FIR for DOA estimation */
    AudioBuffer<float> tmpFir(numActiveInputChannels,firLen);
    BeamParameters tmpBeamParams{0,0};
    for (auto dirIdx = 0;dirIdx < numDoas;dirIdx++){
        tmpBeamParams.doa = -1+(2./(numDoas-1)*dirIdx);
        b.getFir(tmpFir, tmpBeamParams,1);
        doaFirFFT[dirIdx] = FIR::AudioBufferFFT(numActiveInputChannels,fft);
        doaFirFFT[dirIdx].setTimeSeries(tmpFir);
        doaFirFFT[dirIdx].prepareForConvolution();
    }
}

BeamformerDoa::~BeamformerDoa(){
}

void BeamformerDoa::timerCallback(){
    
    beamformer.getDoaInputBuffer(inputBuffer);
    
    /** Compute DOA levels */
    for (auto dirIdx = 0; dirIdx < numDoas; dirIdx++){
        doaBeam.clear();
        for (auto inCh=0;inCh<inputBuffer.getNumChannels();inCh++){
            /** Convolve inputs and DOA FIR */
            convolutionBuffer.convolve(0, inputBuffer, inCh, doaFirFFT[dirIdx], inCh);
            convolutionBuffer.addTimeSeries(0, doaBeam, 0);
        } 
        
        const Range<float> minMax = FloatVectorOperations::findMinAndMax(doaBeam.getReadPointer(0), doaBeam.getNumSamples());
        const float dirEnergy = jmax(abs(minMax.getStart()),abs(minMax.getEnd()));
        const float dirEnergyDb = Decibels::gainToDecibels(dirEnergy);
        doaLevels[dirIdx] = dirEnergyDb;
    }
    
    beamformer.setDoaEnergy(doaLevels);

}

// ==============================================================================
Beamformer::Beamformer(int numBeams_, int numDoas_){
    
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
    
    /** Allocate DOA input buffer */
    doaInputBuffer.setSize(numActiveInputChannels, maximumExpectedSamplesPerBlock);
    doaInputBuffer.clear();
    
    /** Set DOA input Filter  */
    doaBPFilters.clear();
    doaBPFilters.resize(numActiveInputChannels);
    IIRCoefficients doaIIRCoeff = IIRCoefficients::makeBandPass(sampleRate,doaBPfreq, doaBPQ);
    for (auto &f : doaBPFilters){
        f.setCoefficients(doaIIRCoeff);
    }
    
    /** Prepare and start DOA thread */
    doaThread = std::make_unique<BeamformerDoa>(*this,numDoas,sampleRate,numActiveInputChannels,firLen,fft);
    doaThread->startTimerHz(doaUpdateFrequency);
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
    
    {
        GenericScopedLock<SpinLock> lock(doaInputBufferLock);
        for (auto chIdx = 0;chIdx <numActiveInputChannels;chIdx++){
            doaInputBuffer.copyFrom(chIdx, 0, inBuffer, chIdx, 0, inBuffer.getNumSamples());
            doaBPFilters[chIdx].processSamples(doaInputBuffer.getWritePointer(chIdx), inBuffer.getNumSamples());
        }
    }
    
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

void Beamformer::getFir(AudioBuffer<float>&fir,const BeamParameters& params,float alpha) const{
    alg->getFir(fir, params, alpha);
}

void Beamformer::getDoaInputBuffer(FIR::AudioBufferFFT& dst) const{
    GenericScopedLock<SpinLock> lock(doaInputBufferLock);
    dst.setTimeSeries(doaInputBuffer);
    dst.prepareForConvolution();
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

void Beamformer::setDoaEnergy(const std::vector<float>& energy){
    GenericScopedLock<SpinLock> lock(doaLock);
    doaLevels = energy;
}

void Beamformer::getDoaEnergy(std::vector<float>& outDoaLevels) const{
    GenericScopedLock<SpinLock> lock(doaLock);
    outDoaLevels = doaLevels;
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
    
    /** Stop DOA thread */
    doaThread.reset();
    
}
