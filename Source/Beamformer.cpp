/*
 Beamforming processing class
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
 */

#include "Beamformer.h"

#define NUM_DOAX 25
#define NUM_DOAY 9

BeamformerDoa::BeamformerDoa(Beamformer &b,
                             int numDoaHor_,
                             int numDoaVer_,
                             float sampleRate_,
                             int numActiveInputChannels,
                             int firLen,
                             float expectedRate,
                             std::shared_ptr<dsp::FFT> fft_) : Thread("DOA"), beamformer(b) {
    
    numDoaHor = numDoaHor_;
    numDoaVer = numDoaVer_;
    fft = fft_;
    sampleRate = sampleRate_;
    doaUpdateFrequency = expectedRate;
    
    /** Initialize levels and FIR */
    doaLevels.resize(numDoaVer,numDoaHor);
    doaLevels.setConstant(-100);
    newDoaLevels.resize(numDoaVer,numDoaHor);
    newDoaLevels.setConstant(-100);
    doaFirFFT.resize(numDoaHor*numDoaVer);
    
    /** Time constants */
    alpha = 1 - exp(-(1/doaUpdateFrequency) / timeConst);
    
    /** Allocate inputBuffer */
    inputBuffer = AudioBufferFFT(numActiveInputChannels, fft);
    
    /** Allocate convolution buffer */
    convolutionBuffer = AudioBufferFFT(1, fft);
    
    /* Determine frequency bins for energy average */
    lowFreqIdx = lowFreq/sampleRate*fft->getSize();
    numFreqBins = highFreq/sampleRate*fft->getSize() - lowFreqIdx;
    
    /** Compute FIR for DOA estimation */
    AudioBuffer<float> tmpFir(numActiveInputChannels, firLen);
    BeamParameters tmpBeamParams{0,0,0};
    for (auto vDirIdx = 0; vDirIdx < numDoaVer; vDirIdx++) {
        if (numDoaVer > 1){
            tmpBeamParams.doaY = -1 + (2. / (numDoaVer - 1) * vDirIdx);
        }
        for (auto hDirIdx = 0; hDirIdx < numDoaHor; hDirIdx++) {
            tmpBeamParams.doaX = -1 + (2. / (numDoaHor - 1) * hDirIdx);
            b.getFir(tmpFir, tmpBeamParams, 1);
            auto dirIdx = vDirIdx * numDoaHor + hDirIdx;
            doaFirFFT[dirIdx] = AudioBufferFFT(numActiveInputChannels, fft);
            doaFirFFT[dirIdx].setTimeSeries(tmpFir);
            doaFirFFT[dirIdx].prepareForConvolution();
        }
    }
}

void BeamformerDoa::run() {
    
    while (!threadShouldExit()){
        
        /* Wait for previous doa to be consumed before computing a new one */
        while (!threadShouldExit() && beamformer.isDoaOutputBufferNew())
            sleep(5);
        if (threadShouldExit())
            return;
            
        const auto startTick = Time::getHighResolutionTicks();
        
        beamformer.getDoaInputBuffer(inputBuffer);
        
        /** Compute DOA levels */
        for (auto vDirIdx = 0; vDirIdx < numDoaVer; vDirIdx++) {
            for (auto hDirIdx = 0; hDirIdx < numDoaHor; hDirIdx++) {
                auto dirIdx = vDirIdx * numDoaHor + hDirIdx;
                
                convolutionBuffer.clear();
                for (auto inCh = 0; inCh < inputBuffer.getNumChannels(); inCh++) {
                    /** Convolve inputs and DOA FIR and sum*/
                    convolutionBuffer.convolveAndAdd(0, inputBuffer, inCh, doaFirFFT[dirIdx], inCh);
                }
                
                /** Back to regular FFT data */
                convolutionBuffer.updateSymmetricFrequency();
                
                std::complex<float>* cplxData = (std::complex<float>*)convolutionBuffer.getReadPointer(0);
                Eigen::Map<CplxVec> doaBeamMap(cplxData,convolutionBuffer.getNumSamples()/2);

                const float dirEnergy = doaBeamMap.segment(lowFreqIdx,numFreqBins).array().abs().sum()/float(numFreqBins);
                const float dirEnergyDb = Decibels::gainToDecibels(dirEnergy);
                newDoaLevels(vDirIdx,hDirIdx) = dirEnergyDb;
            }
        }
        doaLevels = (doaLevels * (1 - alpha)) + (newDoaLevels * alpha);
        beamformer.setDoaEnergy(doaLevels);
        
        const auto endTick = Time::getHighResolutionTicks();
        const float elapsedTime = Time::highResolutionTicksToSeconds(endTick-startTick);
        const float expectedPeriod = 1.f/doaUpdateFrequency;
        const float sleepTime = expectedPeriod-elapsedTime;
        if (sleepTime > 0){
            sleep(expectedPeriod-elapsedTime);
        }else{
            //TODO: can't keep up, reduce complexity
        }
        
    }
}

BeamformerDoa::~BeamformerDoa(){
    
}

// ==============================================================================
Beamformer::Beamformer(int numBeams_, MicConfig mic, double sampleRate_, int maximumExpectedSamplesPerBlock_,float doaRefreshRate) {
    
    numBeams = numBeams_;
    numDoaVer = isLinearArray(mic) ? 1 : NUM_DOAY;
    numDoaHor = NUM_DOAX;
    micConfig = mic;
    sampleRate = sampleRate_;
    maximumExpectedSamplesPerBlock = maximumExpectedSamplesPerBlock_;
    
    /** Alpha for FIR update */
    alpha = 1 - exp(-(maximumExpectedSamplesPerBlock / sampleRate) / firUpdateTimeConst);
    
    firIR.resize(numBeams);
    firFFT.resize(numBeams);
    
    /** Distance between microphones in eSticks*/
    const float micDistX = 0.03;
    const float micDistY = 0.03;
    
    /** Determine configuration parameters */
    switch (micConfig) {
        case ULA_1ESTICK:
            numMic = 16;
            numRows = 1;
            break;
        case ULA_2ESTICK:
            numMic = 32;
            numRows = 1;
            break;
        case URA_2ESTICK:
            numMic = 32;
            numRows = 2;
            break;
        case ULA_3ESTICK:
            numMic = 48;
            numRows = 1;
            break;
        case URA_3ESTICK:
            numMic = 48;
            numRows = 3;
            break;
        case ULA_4ESTICK:
            numMic = 64;
            numRows = 1;
            break;
        case URA_4ESTICK:
            numMic = 64;
            numRows = 4;
            break;
        case URA_2x2ESTICK:
            numMic = 64;
            numRows = 2;
            break;
    }
    alg = std::make_unique<DAS::FarfieldURA>(micDistX, micDistY, numMic, numRows, sampleRate, soundspeed);
    
    firLen = alg->getFirLen();
    
    /** Create shared FFT object */
    fft = std::make_shared<juce::dsp::FFT>(ceil(log2(firLen + maximumExpectedSamplesPerBlock - 1)));
    
    /** Allocate FIR filters */
    for (auto &f : firIR) {
        f = AudioBuffer<float>(numMic, firLen);
        f.clear();
    }
    for (auto &f : firFFT) {
        f = AudioBufferFFT(numMic, fft);
        f.clear();
    }
    
    /** Allocate input buffers */
    inputBuffer = AudioBufferFFT(numMic, fft);
    
    /** Allocate convolution buffer */
    convolutionBuffer = AudioBufferFFT(1, fft);
    
    /** Allocate beam output buffer */
    beamBuffer.setSize(numBeams, convolutionBuffer.getNumSamples() / 2);
    beamBuffer.clear();
    
    /** Allocate DOA input buffer */
    doaInputBuffer = AudioBufferFFT(numMic, fft);
    doaInputBuffer.prepareForConvolution();
    
    /** Prepare and start DOA thread */
    doaThread = std::make_unique<BeamformerDoa>(*this, numDoaHor, numDoaVer, sampleRate, numMic, firLen, doaRefreshRate, fft);
    doaThread->startThread();
    
}

Beamformer::~Beamformer() {
    doaThread->stopThread(3000);
}

MicConfig Beamformer::getMicConfig() const {
    return micConfig;
}


void Beamformer::setBeamParameters(int beamIdx, const BeamParameters &beamParams) {
    if (alg == nullptr)
        return;
    alg->getFir(firIR[beamIdx], beamParams, alpha);
    firFFT[beamIdx].setTimeSeries(firIR[beamIdx]);
    firFFT[beamIdx].prepareForConvolution();
}

void Beamformer::processBlock(const AudioBuffer<float> &inBuffer) {
    
    /** Compute inputs FFT */
    inputBuffer.setTimeSeries(inBuffer);
    inputBuffer.prepareForConvolution();
    
    if (!doaInputBufferNew){
        GenericScopedLock<SpinLock> lock(doaInputBufferLock);
        doaInputBuffer = inputBuffer;
        doaInputBufferNew = true;
    }
    
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++) {
        for (auto inCh = 0; inCh < inputBuffer.getNumChannels(); inCh++) {
            /** Convolve inputs and FIR */
            convolutionBuffer.convolve(0, inputBuffer, inCh, firFFT[beamIdx], inCh);
            /** Overlap and add of convolutionBuffer into beamBuffer */
            convolutionBuffer.addToTimeSeries(0, beamBuffer, beamIdx);
        }
    }
    
}

void Beamformer::getFir(AudioBuffer<float> &fir, const BeamParameters &params, float alpha) const {
    alg->getFir(fir, params, alpha);
}

void Beamformer::getDoaInputBuffer(AudioBufferFFT &dst) {
    GenericScopedLock<SpinLock> lock(doaInputBufferLock);
    dst = doaInputBuffer;
    doaInputBufferNew = false;
}

void Beamformer::getBeams(AudioBuffer<float> &outBuffer) {
    jassert(outBuffer.getNumChannels() == numBeams);
    auto numSplsOut = outBuffer.getNumSamples();
    auto numSplsShift = beamBuffer.getNumSamples() - numSplsOut;
    for (auto beamIdx = 0; beamIdx < numBeams; beamIdx++) {
        /** Copy beamBuffer to outBuffer */
        outBuffer.copyFrom(beamIdx, 0, beamBuffer, beamIdx, 0, numSplsOut);
        /** Shift beamBuffer */
        FloatVectorOperations::copy(beamBuffer.getWritePointer(beamIdx),
                                    beamBuffer.getReadPointer(beamIdx) + numSplsOut, numSplsShift);
        beamBuffer.clear(beamIdx, numSplsShift, beamBuffer.getNumSamples() - numSplsShift);
    }
}

void Beamformer::setDoaEnergy(const Mtx &energy) {
    GenericScopedLock<SpinLock> lock(doaLock);
    doaLevels = energy;
    doaOutputBufferNew = true;
}

void Beamformer::getDoaEnergy(Mtx &outDoaLevels) {
    GenericScopedLock<SpinLock> lock(doaLock);
    outDoaLevels = doaLevels;
    doaOutputBufferNew = false;
}

MemoryBlock Beamformer::getDoaEnergy(){
    GenericScopedLock<SpinLock> lock(doaLock);
    
    MemoryBlock mb(doaLevels.size()*sizeof(float)+2);
    mb[0] = (uint8)doaLevels.rows();
    mb[1] = (uint8)doaLevels.cols();
    mb.copyFrom(doaLevels.data(), 2, doaLevels.size()*sizeof(float));
    
    doaOutputBufferNew = false;
    return mb;
}

bool Beamformer::isDoaOutputBufferNew() const{
    return doaOutputBufferNew;
}
