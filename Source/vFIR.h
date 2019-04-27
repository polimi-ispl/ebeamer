/*
  ==============================================================================

    vFIR.h
    Created: 27 Apr 2019 9:45:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace vFIR{
// Convolution operations
    void prepareForConvolution (float *samples, int fftSize) noexcept;
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize);
    void updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept;
}
