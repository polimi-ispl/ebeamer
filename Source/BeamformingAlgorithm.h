/*
  ==============================================================================

    BeamformingAlgorithm.h
    Created: 15 Apr 2020 5:48:24pm
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

/** Beam parameters data structure for a linear 1D array */
typedef struct{
    /** DIrection of arrival of the beam.
     Range: -1 (source closer to first microphone) to +1 (source closer to last microphone)
     */
    float doa;
    /** Width of the beam.
     Range: 0 (the most focused) to 1 (the least focused)
     */
    float width;
} BeamParameters;



/** Virtual class extended by all beamforming algorithms
 
 This class is used do setup a LMA and compute the FIR impulse response
 */
class BeamformingAlgorithm{
    
public:
    
    virtual ~BeamformingAlgorithm(){};
    
    /** Get the minimum FIR length for the given configuration [samples] */
    virtual int getFirLen() const = 0;
    
    /** Get FIR in time domain for a given direction of arrival
     
     @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
     @param params: beam parameters
     @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
     */
    virtual void getFir(AudioBuffer<float>&fir,const BeamParameters& params,float alpha=1) const = 0;

};
