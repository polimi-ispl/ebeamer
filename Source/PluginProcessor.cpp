/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JucebeamAudioProcessor::JucebeamAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
                        fft(FFT_ORDER)
#endif
{
    
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

//==============================================================================
void JucebeamAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    olaBuffer = AudioBuffer<float>(getTotalNumOutputChannels(),FFT_SIZE);
    olaBuffer.clear();
    
}

void JucebeamAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool JucebeamAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void JucebeamAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    int blockNumSamples = buffer.getNumSamples();
    
    for (int inChannel = 0; inChannel < totalNumInputChannels; ++inChannel)
    {
        
        // Fill fft data buffer
        FloatVectorOperations::clear(fftInput, 2*FFT_SIZE);
        FloatVectorOperations::copy(fftInput, buffer.getReadPointer(inChannel),blockNumSamples);
        
        // Forward channel FFT
        fft.performRealOnlyForwardTransform(fftInput,true);
        
        // Output channel dependent processing
        for (int outChannel = 0; outChannel < totalNumOutputChannels; ++outChannel)
        {
            FloatVectorOperations::copy(fftOutput,fftInput,2*FFT_SIZE);
            if (inChannel == outChannel)
            {
                
                // (inChannel,outChannel) specific processing
                if (hpEnable)
                {
                    FloatVectorOperations::clear(fftOutput, FFT_SIZE>>3);
                }
                
                // Inverse FFT
                fft.performRealOnlyInverseTransform(fftOutput);
                
                if (bypass)
                {
                    // OLA
                    olaBuffer.addFrom(outChannel, 0, buffer, inChannel, 0,  blockNumSamples);
                }
                else
                {
                    // OLA
                    olaBuffer.addFrom(outChannel, 0, fftOutput, FFT_SIZE);
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
        FloatVectorOperations::copy(olaBuffer.getWritePointer(outChannel), &(olaBuffer.getReadPointer(outChannel)[blockNumSamples]), FFT_SIZE-blockNumSamples);
        olaBuffer.clear(outChannel, FFT_SIZE-blockNumSamples, blockNumSamples);
        
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

