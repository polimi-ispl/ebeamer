/*
  Signal processing utilities
 
 Authors:
 Luca Bondi (luca.bondi@polimi.it)
*/

#pragma once

#include "../Eigen/Eigen"
#include "../JuceLibraryCode/JuceHeader.h"

typedef Eigen::Matrix<float, Eigen::Dynamic, 1> Vec;
typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Mtx;
typedef Eigen::Matrix<std::complex<float>, Eigen::Dynamic, 1> CpxVec;
typedef Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic> CpxMtx;

const float pi = MathConstants<float>::pi;
const Eigen::scomplex j2pi = std::complex<float>(0, 2 * pi);

/** Design a Tukey window
 
 @param win: the destination array for the window
 @param winLen: length of the window [samples]
 @param rampLen: length of cosine ramp [samples]
 */
void designTukeyWindow(Vec &win, size_t winLen, size_t rampLen);

/** Convert a frequency domain signal to a time domain signal.
 
 Optionally apply windowing and exponential smoothing.
 @param time: Destination time domain buffer
 @param timeCh:Time domain buffer destination channel
 @param freq: Source frequency domain signal
 @param fft: FFT object reference
 @param window: a windowing funciton in the time domain
 @param alpha: exponential interpolation coefficient. 1 means complete override (instant update), 0 means no override (complete preservation)
 
 */
void freqToTime(AudioBuffer<float> &time, const int timeCh, const CpxVec &freq, const juce::dsp::FFT *fft,
                const Vec &window = Vec(), float alpha = 1);
