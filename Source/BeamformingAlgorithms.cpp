/*
  Beamforming algorithms
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include "BeamformingAlgorithms.h"

namespace DAS {

    FarfieldURA::FarfieldURA(float micDistX_, float micDistY_,
                             int numMic_, int numRows_, float fs_, float soundspeed_) {

        micDistX = micDistX_;
        micDistY = micDistY_;
        numMic = numMic_;
        numRows = numRows_;
        numMicPerRow = numMic/numRows;
        fs = fs_;
        soundspeed = soundspeed_;

        commonDelay = 64;
        firLen = ceil(jmax(numMic/numRows * micDistX,numRows * micDistY) / soundspeed * fs) + 2 * commonDelay;

        fft = std::make_unique<juce::dsp::FFT>(ceil(log2(firLen)));

        win.resize(fft->getSize());
        designTukeyWindow(win, fft->getSize(), commonDelay / 2);

        freqAxes = Vec::LinSpaced(fft->getSize(), 0, fs * (fft->getSize() - 1) / fft->getSize());

    }

    int FarfieldURA::getFirLen() const {
        return firLen;
    }

    void FarfieldURA::getFir(AudioBuffer<float> &fir, const BeamParameters &params, float alpha) const {

        /** Angle in radians (0 front, pi/2 source closer to last channel, -pi/2 source closer to first channel */
        const float angleRadX = params.doaX * pi / 2;
        /** Angle in radians (0 front, pi/2 source closer to last channel, -pi/2 source closer to first channel */
        const float angleRadY = params.doaY * pi / 2;
        /** Delay between adjacent microphones [s] */
        const float deltaX = sin(angleRadX) * micDistX / soundspeed;
        /** Delay between adjacent microphones [s] */
        const float deltaY = sin(angleRadY) * micDistY / soundspeed;
        /** Compute delays for each microphone, X component [s] */
        const Vec micDelaysX = deltaX * Vec::LinSpaced(numMicPerRow, 0, numMicPerRow - 1);
        /** Compute delays for each microphone, Y component [s] */
        const Vec micDelaysY = deltaY * Vec::LinSpaced(numRows, 0, numRows - 1);
        /** Matrix of delays. Eigen is column-first.*/
        Mtx micDelaysMtx = micDelaysX.replicate(1,numRows) + micDelaysY.transpose().replicate(numMicPerRow,1);
        /** Vector of delays */
        Eigen::Map<Vec> micDelays(micDelaysMtx.data(),micDelaysMtx.size());
        /** Compensate for minimum delay and apply common delay */
        micDelays.array() += -micDelays.minCoeff() + commonDelay / fs;
        /** Compute the fractional delays in frequency domain */
        CpxMtx irFFT = (-j2pi * freqAxes * micDelays.transpose()).array().exp();


        /** Compute how many microphones are muted at each end */
        const int inactiveMicAtBorderX = roundToInt((numMicPerRow / 2 - 1) * params.width);
        const int inactiveMicAtBorderY = roundToInt((numRows / 2 - 1) * params.width);
        /** Generate the mask of active microphones.  Eigen is column-first.*/
        Mtx micGainsMtx = Mtx::Ones(numMicPerRow,numRows);
        for (auto colIdx = 0; colIdx < numRows; colIdx++){
            if ((colIdx < inactiveMicAtBorderY) || (colIdx>=numRows-inactiveMicAtBorderY)){
                micGainsMtx.col(colIdx).setZero();
            }else{
                micGainsMtx.col(colIdx).head(inactiveMicAtBorderX).array() = 0;
                micGainsMtx.col(colIdx).tail(inactiveMicAtBorderX).array() = 0;
            }
        }
        
        Eigen::Map<Vec> micGains(micGainsMtx.data(),micGainsMtx.size());
        
        /** Normalize the power */
        micGains.array() *= referencePower / micGains.sum();

        /** Apply the gain */
        irFFT = irFFT.cwiseProduct(micGains.transpose().replicate(freqAxes.size(), 1));

        /** Convert  from requency to time domain and add to destination*/
        for (auto micIdx = 0; micIdx < jmin(numMic, fir.getNumChannels()); micIdx++) {
            freqToTime(fir, micIdx, irFFT.col(micIdx), fft.get(), win, alpha);
        }
        /** Clear the remaining FIR, if any */
        for (auto micIdx = jmin(numMic, fir.getNumChannels()); micIdx < fir.getNumChannels(); micIdx++) {
            fir.clear(micIdx, 0, fir.getNumSamples());
        }

    }


}
