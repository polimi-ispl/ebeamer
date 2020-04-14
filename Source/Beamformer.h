/*
  ==============================================================================

    Beamformer.h
    Created: 14 Apr 2020 9:12:12am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "FIR.h"

/** Beam parameters data structure for a linear 1D array */
typedef struct{
    /** DIrection of arrival of the beam.
     Range: -1 (left) to +1 (right), looking at the array from the back
     */
    float doa;
    /** Width of the beam.
     Range: 0 (the most focused) to 1 (the least focused)
     */
    float width;
} BeamParameters;

class Beamformer{
    
public:
    
    /** FIR filters length */
    static const int firLen = 648; //~75 Hz @ 48kHz
    
    /** Initialize the Beamformer with a set of static parameters.
     @param numBeams: number of beams the beamformer has to compute
     @param numDoas: number of directions of arrival to compute the energy
     */
    Beamformer(const AudioProcessor& p, int numBeams, int numDoas);
    /** Destructor. */
    ~Beamformer();
    
    /** Set the parameters before execution.
     
     To be called inside AudioProcessor::prepareToPlay.
     This method allocates the needed buffers and performs the necessary pre-calculations that are dependent
     on sample rate, buffer size and channel configurations.
     */
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock, int numActiveInputChannels);
    
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
    
    /** Copy the estimated energy contribution from the directions of arrival */
    void getDoaEnergy(std::vector<float>& energy);
    
    /** Release not needed resources.
     
     To be called inside AudioProcessor::releaseResources.
     */
    void releaseResources();
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Beamformer);
    
    /** Reference to the AudioProcessor instance */
    const AudioProcessor& processor;
    
    /** Number of beams */
    int numBeams;
    
    /** Number of directions of arrival */
    int numDoas;
    
    /** Maximum integer delay */
    static const int maxDelay = 270; //~2m @ 48kHz
    
    /** Shared FFT pointer */
    std::shared_ptr<juce::dsp::FFT> fft;
    
    /** FIR filters for each beam */
    std::vector<FIR::AudioBufferFFT> fir;
    
    /** Inputs' buffer */
    FIR::AudioBufferFFT inputBuffer;
    
    /** Convolution buffer */
    FIR::AudioBufferFFT convolutionBuffer;
    
    /** Beams' outputs buffer */
    AudioBuffer<float> beamBuffer;
    
};
