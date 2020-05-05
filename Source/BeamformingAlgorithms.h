/*
  Beamforming algorithms
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SignalProcessing.h"
#include "BeamformingAlgorithms.h"

/** Beam parameters data structure for a Uniform Rectangular Array
    Convention used:
    - Array seen from behind
    - Upper-left microphone is number 0
    - eSticks are running along the x axes
    - eSticks are stacked along the y axes, top-most eStick has mic form 0 to 15
 */
typedef struct {
    /** Pointing direction of the beam, x axis.
     Range: -1 (source closer to first microphone, left) to +1 (source closer to last microphone, right)
     */
    float doaX;
    /** Pointing direction of the beam, y axis.
    Range: -1 (source closer to first microphone, top) to +1 (source closer to last microphone, bottom)
    */
    float doaY;
    /** Width of the beam.
     Range: 0 (the most focused) to 1 (the least focused)
     */
    float width;
} BeamParameters;


/** Virtual class extended by all beamforming algorithms */
class BeamformingAlgorithm {

public:

    virtual ~BeamformingAlgorithm() {};

    /** Get the minimum FIR length for the given configuration [samples] */
    virtual int getFirLen() const = 0;

    /** Get FIR in time domain for a given direction of arrival
     
     @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
     @param params: beam parameters
     @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
     */
    virtual void getFir(AudioBuffer<float> &fir, const BeamParameters &params, float alpha = 1) const = 0;

};

/** Delay-And-Sum Beamformers*/
namespace DAS {

/** Farfield Uniform Rectangular Array Beamformer
 
 This class is used do setup a URA and compute the FIR impulse response
 */
    class FarfieldURA : public BeamformingAlgorithm {

    public:

        /** initialize the URA

         @param micDistX: microphones distance on the X axes [m]
         @param micDistY: microphones distance on the Y axes [m]
         @param numMic: total number of microphone capsules
         @param numRows: total number of rows
         @param fs: sampling frequency [Hz]
         @param soundspeed: sampling frequency [m/s]
         */
        FarfieldURA(float micDistX, float micDistY, int numMic, int numRows, float fs, float soundspeed);

        /** Get the minimum FIR length for the given configuration [samples] */
        int getFirLen() const override;

        /** Get FIR in time domain for a given direction of arrival

         @param fir: an AudioBuffer object with numChannels >= number of microphones and numSamples >= firLen
         @param params: beam parameters
         @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
         */
        void getFir(AudioBuffer<float> &fir, const BeamParameters &params, float alpha = 1) const override;

    private:

        /** Distance between microphones, X axes [m] */
        float micDistX;
        
        /** Distance between microphones, Y axes [m] */
        float micDistY;
        
        /** Number of microphones */
        int numMic;
        
        /** Number of rows */
        int numRows;
        
        /** Number of mic per row */
        int numMicPerRow;

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
