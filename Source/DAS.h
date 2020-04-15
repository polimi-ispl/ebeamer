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

namespace DAS{

/** Farfield Linear Microphone Array Delay and Sum beamformer
 
 This class is used do setup a LMA and compute the FIR impulse response
 */
class FarfieldLMA{
    
public:
    
    /** initialize the LMA
     
     @param micDist: microphones distance [m]
     @param numMic: number of microphone capsules
     @param fs: sampling frequency [Hz]
     @param soundspeed: sampling frequency [m/s]
     */
    FarfieldLMA(float micDist,int numMic,float fs,float soundspeed);
    
    /** Get the minimum FIR length for the given configuration [samples] */
    int getFirLen() const;
    
    /** Get FIR in time domain for a given direction of arrival
     
     @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
     @param doa: Direction of Arrival in range [-1,+1].
                -1 means the source is closer to microphone in the first channel
                +1 means the source is closer to microphone in thel last channel
     @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
     */
    void getFir(AudioBuffer<float>&fir,float doa,float alpha=1) const;
    
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
    
};

}
