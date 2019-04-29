/*
  ==============================================================================

    vMimoProcessor.cpp
    Created: 27 Apr 2019 10:19:55am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "vMimoProcessor.h"
#include "Binary/firIR.h"
#include "vFIR.h"

//=======================================================
vMimoProcessor::vMimoProcessor(int samplesPerBlock_,int numInputChannels,int numOutputChannels){
    
    // initialize a proper FFT
    samplesPerBlock = samplesPerBlock_;
    size_t fftOrder = roundToInt (std::log2 (FIR_LEN + samplesPerBlock - 1));
    fft = std::make_unique<dsp::FFT>(fftOrder);
    
    // Initialize firFFTs (already prepared for convolution)
#ifdef BEAMSTEERING_ALG_IDEAL
    firSteeringFFT = prepareIR(vFIR::readFIR(firIR::firDASideal_dat,firIR::firDASideal_datSize));
#else
    firFFT = prepareIR(vFIR::readFIR(firIR::firDASmeasured_dat,firIR::firDASmeasured_datSize));
#endif
    /* With Joe we decided that the way we want the interface to behave is to have
     the eStick facing the user, mic 1 on the left, thus we have to reverse the order of the filters.
     */
    std::reverse(firSteeringFFT.begin(), firSteeringFFT.end());
    firBeamwidthFFT = prepareIR(vFIR::readFIR(firIR::firBeamwidth_dat,firIR::firBeamwidth_datSize));
    
    // Allocate beams output buffers
    beamsBuffer = AudioBuffer<float>(NUM_BEAMS,getFftSize());
    beamsBuffer.clear();
    
    // Allocate multi-channel buffers
    fftInputs = AudioBuffer<float>(numInputChannels,2*getFftSize());
    fftInputs.clear();
    
    // Allocate single channel buffers
    fftBuffers = AudioBuffer<float>(NUM_BUFFERS,2*getFftSize());
    fftBuffers.clear();
    
}

//=======================================================
std::vector<std::vector<std::vector<float>>> vMimoProcessor::prepareIR(const std::vector<std::vector<std::vector<float>>> fir)
{
    std::vector<std::vector<std::vector<float>>> firFFT(fir.size());
    for (size_t angleIdx = 0; angleIdx < fir.size(); ++angleIdx)
    {
        std::vector<std::vector<float>> firFFTAngle(fir[angleIdx].size());
        for (size_t micIdx = 0; micIdx < fir[angleIdx].size(); ++micIdx)
        {
            std::vector<float> firFFTAngleMic(2*getFftSize());
            FloatVectorOperations::clear(firFFTAngleMic.data(), 2*getFftSize());
            FloatVectorOperations::copy(firFFTAngleMic.data(), fir[angleIdx][micIdx].data() , static_cast<int>(fir[angleIdx][micIdx].size()));
            fft -> performRealOnlyForwardTransform(firFFTAngleMic.data());
            vFIR::prepareForConvolution(firFFTAngleMic.data(),getFftSize());
            firFFTAngle [micIdx] = firFFTAngleMic;
        }
        firFFT[angleIdx] = firFFTAngle;
    }
    
    return firFFT;
}

void vMimoProcessor::preComputeInputFFT(const AudioBuffer<float>& input){
    if (! fftInputs.hasBeenCleared()){
        throw RuntimeException("Input FFT already computed. vMimoProcessor::newBlock must be called first.");
    }
    for (int inChannel = 0; inChannel < input.getNumChannels(); ++inChannel)
    {
        fftInputs.copyFrom(inChannel, 0, input, inChannel, 0, samplesPerBlock);
        fft -> performRealOnlyForwardTransform(fftInputs.getWritePointer(inChannel));
        vFIR::prepareForConvolution(fftInputs.getWritePointer(inChannel),getFftSize());
    }
}

void vMimoProcessor::getInputFFT(AudioBuffer<float>& inputFFTcopy){
    if (fftInputs.hasBeenCleared()){
        throw RuntimeException("vMimoProcessor::preComputeInputFFT must be called first.");
    }
    inputFFTcopy = static_cast<const AudioBuffer<float>>(fftInputs); // copy operator
}

void vMimoProcessor::newBlock(){
    fftInputs.clear();
    
    // Shift beam OLA buffer
    for (int beamIdx = 0; beamIdx < beamsBuffer.getNumChannels(); ++beamIdx){
        FloatVectorOperations::copy(beamsBuffer.getWritePointer(beamIdx), &(beamsBuffer.getReadPointer(beamIdx)[samplesPerBlock]), beamsBuffer.getNumSamples()-samplesPerBlock);
        beamsBuffer.clear(beamIdx, beamsBuffer.getNumSamples()-samplesPerBlock, samplesPerBlock);
    }
}

void vMimoProcessor::processBlock(int inChannel,int outChannel,int steeringIdx,int beamWidthIdx){
    
    if (fftInputs.hasBeenCleared()){
        throw RuntimeException("vMimoProcessor::preComputeInputFFT must be called first.");
    }
    
    // Copy input fft into buffer 0
    fftBuffers.clear(0,0,fftBuffers.getNumSamples());
    fftBuffers.copyFrom(0, 0, fftInputs, inChannel, 0, fftInputs.getNumSamples());
    
    // Beam width processing (from buffer 0 to 1)
    fftBuffers.clear(1,0,fftBuffers.getNumSamples());
    vFIR::convolutionProcessingAndAccumulate(fftBuffers.getReadPointer(0),firBeamwidthFFT[beamWidthIdx][inChannel].data(),fftBuffers.getWritePointer(1),getFftSize());
    
    // Beam steering processing  (from buffer 1 to 0)
    fftBuffers.clear(0,0,fftBuffers.getNumSamples());
    vFIR::convolutionProcessingAndAccumulate(fftBuffers.getReadPointer(1),firSteeringFFT[steeringIdx][inChannel].data(),fftBuffers.getWritePointer(0),getFftSize());
    
    // FIR post processing
    vFIR::updateSymmetricFrequencyDomainData(fftBuffers.getWritePointer(0),getFftSize());
    
    // Inverse FFT
    fft -> performRealOnlyInverseTransform(fftBuffers.getWritePointer(0));
    
    // OLA
    beamsBuffer.addFrom(outChannel, 0, fftBuffers.getReadPointer(0), getFftSize());
    
}

AudioBuffer<float> vMimoProcessor::getBeams(){
    return beamsBuffer;
}
