/*
  ==============================================================================

    vMimoProcessor.cpp
    Created: 27 Apr 2019 10:19:55am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "vMimoProcessor.h"
#include "Binary/firIR.h"

//=======================================================
vMimoProcessor::vMimoProcessor(int samplesPerBlock_){
    
    // initialize a proper FFT
    samplesPerBlock = samplesPerBlock_;
    size_t fftOrder = roundToInt(ceil(std::log2 (FIR_LEN + samplesPerBlock - 1)));
    fft = std::make_shared<dsp::FFT>(fftOrder);
    
    // Initialize firFFT
    std::vector<AudioBuffer<float>> tmpFIR;
    
    vFIR::readFIR(tmpFIR,
#ifdef BEAMSTEERING_ALG_IDEAL
                  firIR::firDASideal_dat,firIR::firDASideal_datSize
#else
                  firIR::firDASmeasured_dat,firIR::firDASmeasured_datSize
#endif
                  );
    firSteeringFFT.resize(tmpFIR.size());
    for (auto idx = 0; idx < firSteeringFFT.size(); ++idx){
        firSteeringFFT[idx] = vFIR::AudioBufferFFT(tmpFIR[idx],fft);
        firSteeringFFT[idx].prepareForConvolution();
    }

    vFIR::readFIR(tmpFIR,firIR::firBeamwidth_dat,firIR::firBeamwidth_datSize);
    firBeamwidthFFT.resize(tmpFIR.size());
    for (auto idx = 0; idx < firBeamwidthFFT.size(); ++idx){
        firBeamwidthFFT[idx] = vFIR::AudioBufferFFT(tmpFIR[idx],fft);
        firBeamwidthFFT[idx].prepareForConvolution();
    }
    
    /* With Joe we decided that the way we want the interface to behave is to have
     the eStick facing the user, mic 1 on the left, thus we have to reverse the order of the filters.
     */
    std::reverse(firSteeringFFT.begin(), firSteeringFFT.end());
    
    // Allocate single channel buffers
    fftBuffers = vFIR::AudioBufferFFT(NUM_BUFFERS,fft);
    
    
}

//=======================================================

void vMimoProcessor::newBlock(AudioBuffer<float>& outTimeSeries){
    
    // Shift beam OLA buffer
    for (int idx = 0; idx < outTimeSeries.getNumChannels(); ++idx){
        FloatVectorOperations::copy(outTimeSeries.getWritePointer(idx), &(outTimeSeries.getReadPointer(idx)[samplesPerBlock]), outTimeSeries.getNumSamples()-samplesPerBlock);
        outTimeSeries.clear(idx, outTimeSeries.getNumSamples()-samplesPerBlock, samplesPerBlock);
    }
}

void vMimoProcessor::processBlock(const vFIR::AudioBufferFFT& inFFT, int inChannel, vFIR::AudioBufferFFT& outFFT, int outChannel, int steeringIdx, int beamWidthIdx){
    
    if (outFFT.getNumSamples() != 2*fft->getSize()){
        throw std::runtime_error("outFFT has a wrong number of samples");
    }
    
    // Beam width processing
    if (beamWidthIdx >= 0){
        fftBuffers.convolve(0,inFFT,inChannel, firBeamwidthFFT[beamWidthIdx], inChannel);
    }
    else{
        fftBuffers.copyFrom(0, 0, inFFT, inChannel, 0, inFFT.getNumSamples());
    }
    
    // Beam steering processing
    outFFT.convolve(outChannel,fftBuffers,0, firSteeringFFT[steeringIdx], inChannel);
    
}

void vMimoProcessor::processBlock(const vFIR::AudioBufferFFT& inFFT, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx){
    
    if (outTimeSeries.getNumSamples() < fft->getSize()){
        throw std::runtime_error("outTimeSeries doesn't have enough samples");
    }
    
    vFIR::AudioBufferFFT outFFT(1,fft);
    processBlock(inFFT,inChannel,outFFT,0,steeringIdx,beamWidthIdx);
    
    outFFT.addTimeSeries(0,outTimeSeries,outChannel);
    
}

void vMimoProcessor::processBlock(const AudioBuffer<float>& inTimeSeries, int inChannel, AudioBuffer<float>& outTimeSeries, int outChannel, int steeringIdx, int beamWidthIdx){
    
    vFIR::AudioBufferFFT inFFT(inTimeSeries,fft);
    processBlock(inFFT, inChannel, outTimeSeries,outChannel, steeringIdx, beamWidthIdx);
    
}


