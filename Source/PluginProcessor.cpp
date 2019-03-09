/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "firDASideal.h"
#include "firDASmeasured.h"

//==============================================================================
JucebeamAudioProcessor::JucebeamAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::ambisonic(3), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    // Initialize FFT
    fft = new dsp::FFT(roundToInt (std::log2 (FFT_SIZE)));

    // Initialize firFFTs (already prepared for convolution
    firDASidealFft = prepareIR(firDASideal);
    firDASmeasuredFft = prepareIR(firDASmeasured);

}

JucebeamAudioProcessor::~JucebeamAudioProcessor()
{
}

//==============================================================================
const String JucebeamAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JucebeamAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JucebeamAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JucebeamAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double JucebeamAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JucebeamAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int JucebeamAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JucebeamAudioProcessor::setCurrentProgram (int index)
{
}

const String JucebeamAudioProcessor::getProgramName (int index)
{
    return {};
}

void JucebeamAudioProcessor::changeProgramName (int index, const String& newName)
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JucebeamAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const{
    
    int numInputChannels = layouts.getNumChannels(true,0);
    int numOutputChannels = layouts.getNumChannels(false, 0);
    if( (numInputChannels == 16) and (numOutputChannels == 16) ){
        return true;
    }
    return false;
}
#endif

//==============================================================================
void JucebeamAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    auto bufLen = std::max(FFT_SIZE,samplesPerBlock+(FFT_SIZE - MAX_FFT_BLOCK_LEN));
    olaBuffer = AudioBuffer<float>(getTotalNumOutputChannels(),bufLen);
    olaBuffer.clear();
    
}

void JucebeamAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    
}

void JucebeamAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    int blockNumSamples = buffer.getNumSamples();
    
    // Some algorithm logic
    if (prevAlgorithm != algorithm){
        if (algorithm == DAS_IDEAL)
        {
            firFFT = firDASidealFft;
        }
        else if (algorithm == DAS_MEASURED)
        {
            firFFT = firDASmeasuredFft;
        }
        else{
            firFFT.clear();
        }
        prevAlgorithm = algorithm;
    }
    
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        
        for (auto subBlockIdx = 0;subBlockIdx < std::ceil(float(blockNumSamples)/MAX_FFT_BLOCK_LEN);++subBlockIdx)
        {
            auto subBlockFirstIdx = subBlockIdx * MAX_FFT_BLOCK_LEN;
            auto subBlockLen = std::min(blockNumSamples - subBlockFirstIdx,MAX_FFT_BLOCK_LEN);
            
            // Fill fft data buffer
            FloatVectorOperations::clear(fftInput, 2*FFT_SIZE);
            FloatVectorOperations::copy(fftInput, &(buffer.getReadPointer(inChannel)[subBlockFirstIdx]),subBlockLen);
            
            // Forward channel FFT
            fft -> performRealOnlyForwardTransform(fftInput);
            
            // Output channel dependent processing
            for (int outChannel = 0; outChannel < totalNumOutputChannels; ++outChannel)
            {
                if (bypass)
                {
                    // OLA
                    if (outChannel == inChannel)
                    {
                        olaBuffer.addFrom(outChannel, subBlockFirstIdx, buffer, inChannel, subBlockFirstIdx,  subBlockLen);
                    }
                }
                else
                { // No bypass
                    FloatVectorOperations::copy(fftInputCopy,fftInput,2*FFT_SIZE);
                    
                    if (passThrough)
                    {
                        if (outChannel == inChannel)
                        {
                            // Pass-through processing
                            FloatVectorOperations::copy(fftOutput, fftInputCopy, 2*FFT_SIZE);
                            // Inverse FFT
                            fft -> performRealOnlyInverseTransform(fftOutput);
                            // OLA
                            olaBuffer.addFrom(outChannel, subBlockFirstIdx, fftOutput, FFT_SIZE);
                        }
                    }
                    else{ // no passThrough, real processing here!
                        
                        int steeringIdx = roundToInt(((steeringDirections[outChannel] + 1)/2.)*(firFFT.size()-1));
                        
                        // FIR processing
                        FloatVectorOperations::clear(fftOutput, 2*FFT_SIZE);
                        prepareForConvolution(fftInputCopy);
                        convolutionProcessingAndAccumulate(fftInputCopy,firFFT[steeringIdx][inChannel].data(),fftOutput);
                        updateSymmetricFrequencyDomainData(fftOutput);
                        // Inverse FFT
                        fft -> performRealOnlyInverseTransform(fftOutput);
                        // OLA
                        olaBuffer.addFrom(outChannel, subBlockFirstIdx, fftOutput, FFT_SIZE);
                    }
                }
                
            }
            
        }
        
    }
    
    // Prepare OLA output
    for (int outChannel = 0; outChannel < totalNumOutputChannels; ++outChannel)
    {
        // Copy to buffer
        buffer.copyFrom(outChannel, 0, olaBuffer, outChannel, 0, blockNumSamples);
        
        // Shift OLA buffer
        FloatVectorOperations::copy(olaBuffer.getWritePointer(outChannel), &(olaBuffer.getReadPointer(outChannel)[blockNumSamples]), olaBuffer.getNumSamples()-blockNumSamples);
        olaBuffer.clear(outChannel, olaBuffer.getNumSamples()-blockNumSamples, blockNumSamples);
        
    }
}

//==============================================================================
bool JucebeamAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* JucebeamAudioProcessor::createEditor()
{
    return new JucebeamAudioProcessorEditor (*this);
}

//==============================================================================
void JucebeamAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void JucebeamAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JucebeamAudioProcessor();
}

//=======================================================
std::vector<std::vector<std::vector<float>>> JucebeamAudioProcessor::prepareIR(const std::vector<const std::vector<const std::vector<const float>>> fir)
{
    std::vector<std::vector<std::vector<float>>> firFFT(fir.size());
    for (size_t angleIdx = 0; angleIdx < fir.size(); ++angleIdx)
    {
        std::vector<std::vector<float>> firFFTAngle(fir[angleIdx].size());
        for (size_t micIdx = 0; micIdx < fir[angleIdx].size(); ++micIdx)
        {
            std::vector<float> firFFTAngleMic(2*FFT_SIZE);
            FloatVectorOperations::clear(firFFTAngleMic.data(), 2*FFT_SIZE);
            FloatVectorOperations::copy(firFFTAngleMic.data(), fir[angleIdx][micIdx].data() , static_cast<int>(fir[angleIdx][micIdx].size()));
            fft -> performRealOnlyForwardTransform(firFFTAngleMic.data());
            prepareForConvolution(firFFTAngleMic.data());
            firFFTAngle [micIdx] = firFFTAngleMic;
        }
        firFFT[angleIdx] = firFFTAngle;
    }
    
    return firFFT;
}

//========== copied from juce_Convolution.cpp ============

/** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
void JucebeamAudioProcessor::prepareForConvolution (float *samples) noexcept
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    for (size_t i = 0; i < FFTSizeDiv2; i++)
        samples[i] = samples[2 * i];
    
    samples[FFTSizeDiv2] = 0;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
        samples[i + FFTSizeDiv2] = -samples[2 * (FFT_SIZE - i) + 1];
}

/** Does the convolution operation itself only on half of the frequency domain samples. */
void JucebeamAudioProcessor::convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output)
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
    FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));
    
    output[FFT_SIZE] += input[FFT_SIZE] * impulse[FFT_SIZE];
}

/** Undo the re-organization of samples from the function prepareForConvolution.
 Then, takes the conjugate of the frequency domain first half of samples, to fill the
 second half, so that the inverse transform will return real samples in the time domain.
 */
void JucebeamAudioProcessor::updateSymmetricFrequencyDomainData (float* samples) noexcept
{
    auto FFTSizeDiv2 = FFT_SIZE / 2;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * (FFT_SIZE - i)] = samples[i];
        samples[2 * (FFT_SIZE - i) + 1] = -samples[FFTSizeDiv2 + i];
    }
    
    samples[1] = 0.f;
    
    for (size_t i = 1; i < FFTSizeDiv2; i++)
    {
        samples[2 * i] = samples[2 * (FFT_SIZE - i)];
        samples[2 * i + 1] = -samples[2 * (FFT_SIZE - i) + 1];
    }
}

