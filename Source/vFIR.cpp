/*
  ==============================================================================

    vFIR.cpp
    Created: 27 Apr 2019 9:45:28am
    Author:  Luca Bondi

  ==============================================================================
*/

#include "vFIR.h"


namespace vFIR{
//========== copied from juce_Convolution.cpp ============

/** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
    void prepareForConvolution (float *samples, int fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[2 * i];
    
    samples[FFTSizeDiv2] = 0;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[2 * (fftSize - i) + 1];
}

/** Does the convolution operation itself only on half of the frequency domain samples. */
void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output, int fftSize)
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));
    
    output[fftSize] += input[fftSize] * impulse[fftSize];
}

/** Undo the re-organization of samples from the function prepareForConvolution.
 Then, takes the conjugate of the frequency domain first half of samples, to fill the
 second half, so that the inverse transform will return real samples in the time domain.
 */
void updateSymmetricFrequencyDomainData (float* samples, int fftSize) noexcept
{
    auto FFTSizeDiv2 = fftSize / 2;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * (fftSize - i)] = samples[i];
        samples[2 * (fftSize - i) + 1] = -samples[FFTSizeDiv2 + i];
    }
    
    samples[1] = 0.f;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * i] = samples[2 * (fftSize - i)];
        samples[2 * i + 1] = -samples[2 * (fftSize - i) + 1];
    }
}

}
