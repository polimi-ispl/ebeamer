/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#define FFT_ORDER 8 // 10: 1024 samples
#define FFT_SIZE (1 << FFT_ORDER)
#define MAX_FFT_BLOCK_LEN (1 << (FFT_ORDER - 1))
#define MAX_INPUT_CHANNELS 2
#define MAX_OUTPUT_CHANNELS 2

//==============================================================================
/**
*/
class JucebeamAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JucebeamAudioProcessor();
    ~JucebeamAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Project specific
    bool hpEnable = 0;
    bool bypass = 0;
    float steeringDirection = 0;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucebeamAudioProcessor)
    
    // Project specific
    dsp::FFT fft;
    float fftInput[2 * FFT_SIZE];
    float fftOutput[2 * FFT_SIZE];
    AudioBuffer<float> olaBuffer;
    
};
