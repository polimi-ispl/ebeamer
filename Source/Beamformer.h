/*
 ==============================================================================
 
 Beamformer.h
 Created: 14 Apr 2020 9:12:12am
 Author:  Luca Bondi
 
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BeamformingAlgorithm.h"
#include "FIR.h"
#include "DAS.h"

/** Available configurations */
typedef enum {
    LMA_1ESTICK,
    LMA_2ESTICK,
    LMA_3ESTICK,
    LMA_4ESTICK,
} MicConfig;

// ==============================================================================

/** Pre-declare class */
class Beamformer;

/** Separate thread used to compute DOA */
class BeamformerDoa: public Timer{
public:

    BeamformerDoa(Beamformer& b,
                  int numDoas_,
                  float sampleRate_,
                  int numActiveInputChannels,
                  int firLen,
                  std::shared_ptr<dsp::FFT> fft_);
    ~BeamformerDoa();
    
    void timerCallback() override;

private:
    
    /** Reference to the Beamformer */
    Beamformer& beamformer;
    
    /** Number of directions of arrival */
    int numDoas;
    
    /** Sampling frequency [Hz] */
    float sampleRate;
    
    /** FFT */
    std::shared_ptr<dsp::FFT> fft;
    
    /** Inputs' buffer */
    FIR::AudioBufferFFT inputBuffer;
    
    /** Convolution buffer */
    FIR::AudioBufferFFT convolutionBuffer;
    
    /** FIR filters for DOA estimation */
    std::vector<FIR::AudioBufferFFT> doaFirFFT;
    
    /** DOA beam */
    AudioBuffer<float> doaBeam;
    
    /** DOA levels [dB] */
    std::vector<float> doaLevels;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamformerDoa);
    
};

// ==============================================================================

class Beamformer{
    
public:
    
    
    /** Initialize the Beamformer with a set of static parameters.
     @param numBeams: number of beams the beamformer has to compute
     @param numDoas: number of directions of arrival to compute the energy
     */
    Beamformer(int numBeams, int numDoas);
    /** Destructor. */
    ~Beamformer();
    
    /** Set microphone configuration */
    void setMicConfig(MicConfig micConfig_);
    
    /** Get microphone configuration */
    MicConfig getMicConfig() const;
    
    /** Set the parameters before execution.
     
     To be called inside AudioProcessor::prepareToPlay.
     This method allocates the needed buffers and performs the necessary pre-calculations that are dependent
     on sample rate, buffer size and channel configurations.
     */
    void prepareToPlay(double sampleRate_, int maximumExpectedSamplesPerBlock_, int numActiveInputChannels);
    
    /** Process a new block of samples.
     
     To be called inside AudioProcessor::processBlock.
     */
    void processBlock(const AudioBuffer<float>& inBuffer);
    
    /** Copy the current beams outputs to the provided output buffer
     
     To be called inside AudioProcessor::processBlock, after Beamformer::processBlock
     */
    void getBeams(AudioBuffer<float>& outBuffer);
       
    /** Set the parameters for a specific beam  */
    void setBeamParameters(int beamIdx, const BeamParameters& beamParams);
    
    /** Get FIR in time domain for a given direction of arrival
    
    @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
    @param params: beam parameters
    @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
    */
    void getFir(AudioBuffer<float>&fir,const BeamParameters& params,float alpha=1) const;
    
    /** Copy the estimated energy contribution from the directions of arrival */
    void getDoaEnergy(std::vector<float>& energy) const;
    
    /** Set the estimated energy contribution from the directions of arrival */
    void setDoaEnergy(const std::vector<float>& energy);
    
    /** Get last doa filtered input buffer */
    void getDoaInputBuffer(FIR::AudioBufferFFT& dst) const;
    
    /** Release not needed resources.
     
     To be called inside AudioProcessor::releaseResources.
     */
    void releaseResources();
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Beamformer);
    
    /** Sound speed [m/s] */
    const float soundspeed = 343;
    
    /** Sample rate [Hz] */
    float sampleRate = 48000;
    
    /** Maximum buffer size [samples] */
    int maximumExpectedSamplesPerBlock = 64;
    
    /** Number of active inputs */
    int numActiveInputChannels = 1;
    
    /** Number of beams */
    int numBeams;
    
    /** Number of directions of arrival */
    int numDoas;
    
    /** Beamforming algorithm */
    std::unique_ptr<BeamformingAlgorithm> alg;
    
    /** FIR filters length. Diepends on the algorithm */
    int firLen;
    
    /** Shared FFT pointer */
    std::shared_ptr<juce::dsp::FFT> fft;
    
    /** FIR filters for each beam */
    std::vector<AudioBuffer<float>> firIR;
    std::vector<FIR::AudioBufferFFT> firFFT;
    
    /** Inputs' buffer */
    FIR::AudioBufferFFT inputBuffer;
    
    /** Convolution buffer */
    FIR::AudioBufferFFT convolutionBuffer;
    
    /** Beams' outputs buffer */
    AudioBuffer<float> beamBuffer;
    
    /** FIR coefficients update alpha */
    const float alpha = 0.02;
    
    /** Microphones configuration */
    MicConfig micConfig = LMA_1ESTICK;
    
    /** Initialize the beamforming algorithm */
    void initAlg();
    
    
    
    /** DOA thread */
    std::unique_ptr<BeamformerDoa> doaThread;
    
    /**DOA update requency [Hz] */
    const float doaUpdateFrequency = 10;
    
    /** DOA levels [dB] */
    std::vector<float> doaLevels;
    
    /** DOA Band pass Filters */
    std::vector<IIRFilter> doaBPFilters;
    const float doaBPfreq = 2000;
    const float doaBPQ = 1;
    
    /** inputBuffer lock */
    SpinLock doaInputBufferLock;
    
    /** Input buffer with DOA-filtered input signal */
    AudioBuffer<float> doaInputBuffer;
    
    /** DOA Lock */
    SpinLock doaLock;
    
    
};
