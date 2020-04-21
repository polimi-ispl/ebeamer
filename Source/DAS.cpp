/*
  ==============================================================================

    DAS.cpp
    Created: 15 Apr 2020 9:21:10am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "DAS.h"

namespace DAS{



FarfieldLMA::FarfieldLMA(float micDist_,int numMic_,float fs_,float soundspeed_){
    
    micDist = micDist_;
    numMic = numMic_;
    fs = fs_;
    soundspeed = soundspeed_;
    
    commonDelay = 64;
    firLen = ceil(numMic*micDist/soundspeed*fs) + 2*commonDelay;
    
    fft = std::make_unique<juce::dsp::FFT>(ceil(log2(firLen)));
    
    win.resize(fft->getSize());
    designTukeyWindow(win, fft->getSize(), commonDelay/2);
    
    freqAxes = Vec::LinSpaced(fft->getSize(),0,fs*(fft->getSize()-1)/fft->getSize());

}

int FarfieldLMA::getFirLen() const{
    return firLen;
}

void FarfieldLMA::getFir(AudioBuffer<float>&fir,const BeamParameters& params,float alpha) const{
       
    /** Angle in radians (0 front, pi/2 source closer to last channel, -pi/2 source closer to first channel */
    const float angleRad = (params.doa+1)*pi/2;
    /** Delay between adjacent microphones [s] */
    const float delta = -cos(angleRad)*micDist/soundspeed;
    /** Compute delays for each microphone [s] */
    Vec micDelays = delta*Vec::LinSpaced(numMic, 0, numMic-1);
    /** Compensate for minimum delay and apply common delay */
    micDelays.array() += - micDelays.minCoeff() + commonDelay/fs;
    /** Compute the fractional delays in frequency domain */
    CpxMtx irFFT = (-j2pi*freqAxes*micDelays.transpose()).array().exp();
    
    
    /** Compute how many microphones are muted at each end */
    const int inactiveMicAtBorder = roundToInt((numMic/2-1)*params.width);
    /** Generate the mask of active microphones */
    Vec micGains = Vec::Ones(numMic);
    micGains.head(inactiveMicAtBorder).array() = 0;
    micGains.tail(inactiveMicAtBorder).array() = 0;
    /** Normalize the power */
    micGains.array() *= referencePower/micGains.sum();
    
    /** Apply the gain */
    irFFT = irFFT.cwiseProduct(micGains.transpose().replicate(freqAxes.size(), 1));
    
    /** Convert  from requency to time domain and add to destination*/
    for (auto micIdx=0;micIdx<jmin(numMic,fir.getNumChannels());micIdx++){
        freqToTime(fir,micIdx,irFFT.col(micIdx),fft.get(),win,alpha);
    }
    /** Clear the remaining FIR, if any */
    for (auto micIdx=jmin(numMic,fir.getNumChannels());micIdx<fir.getNumChannels();micIdx++){
        fir.clear(micIdx, 0, fir.getNumSamples());
    }
    
}
    

}
