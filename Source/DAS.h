/*
  ==============================================================================

    DAS.h
    Created: 15 Apr 2020 9:21:10am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "SigProc.h"
#include "BeamformingAlgorithm.h"


namespace DAS{

/** Farfield Linear Microphone Array Delay and Sum beamformer
 
 This class is used do setup a LMA and compute the FIR impulse response
 */
class FarfieldLMA : public BeamformingAlgorithm{
    
public:
    
    /** initialize the LMA
     
     @param micDist: microphones distance [m]
     @param numMic: number of microphone capsules
     @param fs: sampling frequency [Hz]
     @param soundspeed: sampling frequency [m/s]
     */
    FarfieldLMA(float micDist,int numMic,float fs,float soundspeed);
    
    /** Get the minimum FIR length for the given configuration [samples] */
    int getFirLen() const override;
    
    /** Get FIR in time domain for a given direction of arrival
     
     @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
     @param params: beam parameters
     @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
     */
    void getFir(AudioBuffer<float>&fir,const BeamParameters& params,float alpha=1) const override;
    
private:
    
    /** Distance between microphones [m] */
    float micDist;
    
    /** Number of microphones */
    int numMic;
    
    /** Sampling frequency [Hz] */
    float fs;
    
    /** Soundspeed [m/s] */
    float soundspeed;
    
    /** Common delay applied to all the filters to make filters causal [samples] */
    int commonDelay;
    
    /** Length of FIR filters [samples] */
    int firLen;
    
    /** FFT object */
    std::unique_ptr<juce::dsp::FFT> fft;
    
    /** Window applied to the FIR filters in time domain */
    Vec win;
    
    /** Frequencies axes */
    Vec freqAxes;
    
    /** Reference power for normalization */
    const float referencePower = 3;
    
};

}