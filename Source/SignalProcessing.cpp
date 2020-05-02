/*
  Signal processing utilities
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#include "SignalProcessing.h"


void designTukeyWindow(Vec &win, size_t winLen, size_t rampLen) {
    jassert(winLen <= win.size() && winLen >= 3);
    jassert(rampLen >= 0 && rampLen <= winLen / 2);

    Vec winTmp = Vec::Ones(winLen, 1);

    if (rampLen > 0) {
        winTmp.head(rampLen) =
                ((Vec::LinSpaced(rampLen, -pi, 0)).array().cos() + 1.0f) * 0.5f;
        winTmp.tail(rampLen) = winTmp.head(rampLen).colwise().reverse();
    }
    /** Center the window */
    const size_t offset = roundToInt(floor((win.size() - winLen) / 2.));
    win.setZero();
    win.segment(offset, winLen) = winTmp;

}

void
freqToTime(AudioBuffer<float> &time, const int timeCh, const CpxVec &freq, const juce::dsp::FFT *fft, const Vec &window,
           float alpha) {

    alpha = jlimit(0.f, 1.f, alpha);

    AudioBuffer<float> tmp(1, fft->getSize() * 2);
    tmp.clear();

    FloatVectorOperations::copy(tmp.getWritePointer(0),
                                (float *) (freq.data()),
                                (fft->getSize() / 2 + 1) * 2);
    fft->performRealOnlyInverseTransform(tmp.getWritePointer(0));

    if (window.size()) {
        /** Apply windowing to IR */
        FloatVectorOperations::multiply(tmp.getWritePointer(0), window.data(),
                                        fft->getSize());
    }

    if (alpha < 1) {
        /** Exp smoothing */
        FloatVectorOperations::multiply(time.getWritePointer(timeCh), 1.f - alpha, time.getNumSamples());
        FloatVectorOperations::addWithMultiply(time.getWritePointer(timeCh), tmp.getReadPointer(0), alpha,
                                               time.getNumSamples());
    } else {
        FloatVectorOperations::copy(time.getWritePointer(timeCh), tmp.getReadPointer(0), time.getNumSamples());
    }

}
